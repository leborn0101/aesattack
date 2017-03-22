#ifndef _LIBCACHE_CPUTILS_
#define _LIBCACHE_CPUTILS_

#include "libcache.h"
#include "utils.h"

#define HISTOGRAM_NUM 500
#define HISTOGRAM_GAP 5
#define REPEAT_TIMES 50000
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

typedef enum {
	CORE_256, CORE_512
} core_t;

int get_core_type(libcache_session_t* session, void** array, size_t length, size_t cpu);

#endif
