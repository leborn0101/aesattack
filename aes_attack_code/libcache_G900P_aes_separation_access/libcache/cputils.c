#include <unistd.h>
#include "cputils.h"
#define DEBUG
#define TEST_SET 0
#define TEST_SET_GAP 256 

int get_core_type(libcache_session_t* session, void** array, size_t length, size_t cpu)
{
	uint64_t* time_array_hit;
	uint64_t* time_array_miss;
	uint64_t* time_array;
	size_t available_cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	size_t sum_of_cpu = sysconf(_SC_NPROCESSORS_CONF);
#ifdef DEBUG
	LOGI("available cpu num : %4zu", available_cpu_num);
	LOGI("sum of cpu : %4zu", sum_of_cpu);
#endif
	for (size_t i = 0; i < sum_of_cpu; i++) {
		uint64_t hit_time = 0;
		uint64_t miss_time = 0;
		uint64_t test_time = 0;
		size_t hit_times = 0;
		size_t miss_times = 0;
		size_t test_times = 0;
		time_array = (uint64_t*)calloc(HISTOGRAM_NUM, sizeof(uint64_t));
		if (time_array == NULL) {
			LOGI("Error : Could not allocate memory for time_array!!");
			return -1;
		}
		time_array_hit = (uint64_t*)calloc(HISTOGRAM_NUM, sizeof(uint64_t));
		if (time_array_hit == NULL) {
			LOGI("Error : Could not allocate memory for time_array_hit!!");
			free(time_array);
			return -1;
		}
		memset(time_array_hit, 0, HISTOGRAM_NUM * sizeof(uint64_t));
		time_array_miss = (uint64_t*)calloc(HISTOGRAM_NUM, sizeof(uint64_t));
		if (time_array_miss == NULL) {
			LOGI("Error : Could not allocate memory for time_array_miss!!");
			free(time_array);
			free(time_array_hit);
			return -1;
		}
		memset(time_array_miss, 0, HISTOGRAM_NUM * sizeof(uint64_t));
		if (libcache_bind_to_cpu(i) == false) {
			LOGI("Error : Could not bind thread to cpu %4zu!!", i);
			if (time_array != NULL) {
				free(time_array);
				time_array = NULL;
			}
			if (time_array_hit != NULL) {
				free(time_array_hit);
				time_array_hit = NULL;
			}
			if (time_array_miss != NULL) {
				free(time_array_miss);
				time_array_miss = NULL;
			}
			continue;
		} else {
			LOGI("Now, thread bind to cpu %4zu", i);
		}
		if ((TEST_SET >= length) || (TEST_SET + TEST_SET_GAP >= length)) {
			LOGI("Error : Access array out of bound!!");
			free(time_array);
			free(time_array_hit);
			free(time_array_miss);
			return -1;
		}
		uint64_t time;
		for (int j = 0; j < REPEAT_TIMES; j++) {
			time = libcache_reload_address(session, array[TEST_SET]);
			time_array_hit[MIN(time / HISTOGRAM_GAP, HISTOGRAM_NUM - 1)] += 1;
		} 
		for (int j = 0; j < REPEAT_TIMES; j++) {
			time = libcache_reload_address_and_evict(session, array[TEST_SET]);
			time_array_miss[MIN(time / HISTOGRAM_GAP, HISTOGRAM_NUM - 1)] += 1;
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
		LOGI("cache hit time : %10llu times : %10zu", hit_time, hit_times);
		LOGI("cache miss time : %10llu times : %10zu", miss_time, miss_times);
		for (int j = 0; j < REPEAT_TIMES; j++) {
			time = libcache_reload_address_and_evict_other_address(session, array[TEST_SET], array[TEST_SET + TEST_SET_GAP]);
			time_array[MIN(time / HISTOGRAM_GAP, HISTOGRAM_NUM - 1)] += 1;
		}	
		for (int j = 0; j < HISTOGRAM_NUM; j++) {
			if (time_array[j] > test_times) {
				test_time = j * HISTOGRAM_GAP;
				test_times = time_array[j];
			}
		}	
		LOGI("test time : %10llu times : %10zu", test_time, test_times);
		if (test_time > (hit_time + miss_time) / 2) {
			LOGI("core %4zu bind to CPU_256", i);
		} else {
			LOGI("core %4zu bind to CPU_512", i);
		} 
	}
  if (libcache_bind_to_cpu(cpu) == false) {
    LOGI("Warning: Could not bind to CPU: %zu\n", cpu);
  } else {
		LOGI("attack process bind to cpu %d", cpu);
	}
	if (time_array != NULL)
		free(time_array);
	if (time_array_hit != NULL)
		free(time_array_hit);
	if (time_array_miss != NULL)
		free(time_array_miss);
	return 0;
}
