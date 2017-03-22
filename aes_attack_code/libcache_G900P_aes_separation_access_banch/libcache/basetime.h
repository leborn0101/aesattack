#ifndef _LIBCACHE_CPUTILS_
#define _LIBCACHE_CPUTILS_

#include "libcache.h"
#include "utils.h"
#include <stdio.h>
#include "internal.h"

#define HISTOGRAM_NUM 50
#define HISTOGRAM_GAP 10
#define REPEAT_TIMES 50000
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

int get_base_time(libcache_session_t* session, void** array, size_t length, uint64_t* base_time);
int get_current_basetime(libcache_session_t* session, uint64_t* base_time);
uint64_t get_real_hit_time(libcache_session_t* session);
int get_bhm_time(libcache_session_t* session);

#endif
