#include <unistd.h>
#include "basetime.h"
#define DEBUG
#define TEST_SET 0
#define BASE_TIME_TIMES 100000
#define REAL_HIT_TIMES 100000

uint64_t get_real_hit_time(libcache_session_t* session) {
	uint64_t access_addr = 0;
	uint64_t hit_time = 0;
	uint64_t start = 0, end = 0;
	start = libcache_get_timing_start(session);
	for (int i = 0; i < REAL_HIT_TIMES; i++) {
		libcache_access_memory(&access_addr);
	}
	end = libcache_get_timing_end(session);
	hit_time = (end - start) / REAL_HIT_TIMES;
	return hit_time;
}
	
int get_bhm_time(libcache_session_t* session)
{

	uint64_t time_array_base[BASE_TIME_TIMES];
	uint64_t time_array_hit[BASE_TIME_TIMES];
	uint64_t time_array_miss[BASE_TIME_TIMES];

	uint64_t time;
	uint64_t access = 0;

	FILE* fb;
	if ((fb = fopen("/sdcard/btime", "a")) == NULL) {
		LOGI("Error : could not open /sdcard/btime");
		return -1;
	} else {
		LOGI("open /sdcard/btime success!");
	}

	for (int j = 0; j < BASE_TIME_TIMES; j++) {
		time = libcache_get_basetime(session);
		if ((j % 10000) == 0)
			fprintf(fb, "\n");
		fprintf(fb, "%10llu", time);	
	}
	fclose(fb);

	FILE* fh;
	if ((fh = fopen("/sdcard/htime", "a")) == NULL) {
		LOGI("Error : could not open /sdcard/htime");
		return -1;
	} else {
		LOGI("open /sdcard/htime success!");
	}
	for (int j = 0; j < BASE_TIME_TIMES; j++) {
		time = libcache_reload_address(session, &access);
		if ((j % 10000) == 0)
			fprintf(fh, "\n");
		fprintf(fh, "%10llu", time);
	} 
	fclose(fh);

	FILE* fm;
	if ((fm = fopen("/sdcard/mtime", "a")) == NULL) {
		LOGI("Error : could not open /sdcard/mtime");
		return -1;
	} else {
		LOGI("open /sdcard/mtime success!");
	}
	for (int j = 0; j < BASE_TIME_TIMES; j++) {
		time = libcache_reload_address_and_evict(session, &access);
		if ((j % 10000) == 0)
			fprintf(fm, "\n");
		fprintf(fm, "%10llu", time);
	}
	fclose(fm);

	return 0;
}
int get_base_time(libcache_session_t* session, void** array, size_t length, uint64_t* base_time)
{
	uint64_t* time_array_hit;
	uint64_t* time_array_miss;
	uint64_t time_array_base[BASE_TIME_TIMES];
	uint64_t hit_time = 0;
	uint64_t miss_time = 0;
	size_t hit_times = 0;
	size_t miss_times = 0;
	time_array_hit = (uint64_t*)calloc(HISTOGRAM_NUM, sizeof(uint64_t));
	if (time_array_hit == NULL) {
		LOGI("Error : Could not allocate memory for time_array_hit!!");
		return -1;
	}
	memset(time_array_hit, 0, HISTOGRAM_NUM * sizeof(uint64_t));
	time_array_miss = (uint64_t*)calloc(HISTOGRAM_NUM, sizeof(uint64_t));
	if (time_array_miss == NULL) {
		LOGI("Error : Could not allocate memory for time_array_miss!!");
		free(time_array_hit);
		return -1;
	}
	memset(time_array_miss, 0, HISTOGRAM_NUM * sizeof(uint64_t));
	if (TEST_SET >= length) {
		LOGI("Error : Access array out of bound!!");
		free(time_array_hit);
		free(time_array_miss);
		return -1;
	}
	uint64_t time;
	for (int j = 0; j < BASE_TIME_TIMES * 2; j++) {
		time = libcache_get_basetime(session);
		if (j >= BASE_TIME_TIMES) {
			time_array_base[j - BASE_TIME_TIMES] = time;
		}
	}
	uint64_t basetime = 0;
	uint64_t basetimesum = 0;
	int availablebasetimes = 0;
	uint64_t timesum = 0;
	for (int j = 0; j < BASE_TIME_TIMES; j++) {	
		timesum = timesum + time_array_base[j];
	}
	timesum = timesum / BASE_TIME_TIMES;
	for (int j = 0; j < BASE_TIME_TIMES; j++) {
		if (time_array_base[j] < 2 * timesum) {
			basetimesum += time_array_base[j];
			availablebasetimes++;
		}
	}
	if (availablebasetimes != 0) {
		basetime = basetimesum / availablebasetimes;
		session->basetime = basetime;
	}
	else {
		LOGI("Error : available basetimes equals zero!!!");
		return -1;
	}
#ifdef DEBUG
	LOGI("sum count times : %d", BASE_TIME_TIMES);
	LOGI("basetime : %10llu", basetime);
	LOGI("available count times : %llu", availablebasetimes);
#endif
	for (int j = 0; j < REPEAT_TIMES; j++) {
		time = libcache_reload_address(session, array[TEST_SET]);
		if (time >= basetime) {
			time -= basetime;			
			time_array_hit[MIN(time / HISTOGRAM_GAP, HISTOGRAM_NUM - 1)] += 1;
		}
	} 
	for (int j = 0; j < REPEAT_TIMES; j++) {
		time = libcache_reload_address_and_evict(session, array[TEST_SET]);
		if (time >= basetime) {
			time -= basetime;				
			time_array_miss[MIN(time / HISTOGRAM_GAP, HISTOGRAM_NUM - 1)] += 1;
		}
	}
	for (int j = 0; j < HISTOGRAM_NUM; j++) {
		if (time_array_hit[j] > hit_times) {
			hit_time = j * HISTOGRAM_GAP;
			hit_times = time_array_hit[j];
		}
		if (time_array_miss[j] > miss_times) {
			miss_time = j * HISTOGRAM_GAP;
			miss_times = time_array_miss[j];
		}
	}
	/*FILE* fp;
	if ((fp = fopen("/sdcard/basetime", "a") == NULL)) {
		LOGI("Error : could not open /sdcard/basetime");
		return -1;
	} else {
		LOGI("open /sdcard/basetime success!");
	}
	fprintf(fp, "histogram scale : %d, histogram num : %d\n", HISTOGRAM_GAP, HISTOGRAM_NUM);
	for (int j = 0; j < HISTOGRAM_NUM; j++) {
		fprintf(fp, "%10zu", time_array_hit[j]);
	}
	fprintf(fp, "\n");
	for (int j = 0; j < HISTOGRAM_NUM; j++) {
		fprintf(fp, "%10zu", time_array_miss[j]);
	}
	fprintf(fp, "\n");
	*/
	LOGI("real hit time : %10llu", get_real_hit_time(session));
	LOGI("cache hit time : %10llu times : %10zu", hit_time, hit_times);
	LOGI("cache miss time : %10llu times : %10zu", miss_time, miss_times);
	if (time_array_hit != NULL)
		free(time_array_hit);
	if (time_array_miss != NULL)
		free(time_array_miss);
	//fclose(fp);
	*base_time = basetime;
	return 0;
}

int get_current_basetime(libcache_session_t* session, uint64_t* base_time)
{
	FILE* fp;
	if ((fp = fopen("/sdcard/realtimebasetime", "a")) == NULL) {
		LOGI("Error : could not open /sdcard/realtimebasetime");
		return -1;
	} else {
		LOGI("open /sdcard/realtimebasetime success!");
	}
	
	uint64_t time_array_base[BASE_TIME_TIMES];
	uint64_t time = 0;
	for (int j = 0; j < BASE_TIME_TIMES * 2; j++) {
		time = libcache_get_basetime(session);
		if (j < BASE_TIME_TIMES) {
			fprintf(fp, "%10llu", time);
		}

		if (j >= BASE_TIME_TIMES) {
			time_array_base[j - BASE_TIME_TIMES] = time;
		}
	}
	uint64_t basetime = 0;
	uint64_t basetimesum = 0;
	int availablebasetimes = 0;
	uint64_t timesum = 0;
	for (int j = 0; j < BASE_TIME_TIMES; j++) {	
		timesum = timesum + time_array_base[j];
	}
	timesum = timesum / BASE_TIME_TIMES;
	for (int j = 0; j < BASE_TIME_TIMES; j++) {
		if (time_array_base[j] < timesum * 2) {
			basetimesum += time_array_base[j];
			availablebasetimes++;
		}
	}
	if (availablebasetimes != 0) {
		basetime = basetimesum / availablebasetimes;
		session->basetime = basetime;
		*base_time = basetime;
	} else {
		LOGI("Error : available basetimes equals zero!!!");
		return -1;
	}
#ifdef DEBUG
	LOGI("basetime : %10llu(sum count times : %d, available count times : %d)", basetime, BASE_TIME_TIMES, availablebasetimes);
#endif
	fclose(fp);
	return 0;
}
