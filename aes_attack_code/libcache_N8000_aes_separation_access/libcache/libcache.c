/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <sched.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libcache.h"
#include "timing.h"
#include "internal.h"
#include "eviction/eviction.h"

#include <pthread.h>

#if defined(__ARM_ARCH_7A__)
#include "armv7/libcache.h"
#elif defined(__ARM_ARCH_8A__)
#include "armv8/libcache.h"
#endif

uint64_t libcache_get_timing_start(libcache_session_t* session);
uint64_t libcache_get_timing_end(libcache_session_t* session);

static size_t get_frame_number_from_pagemap(size_t value);

bool
libcache_init(libcache_session_t** session, libcache_session_args_t* args)
{
  (void) session;
  (void) args;

  if (session == NULL) {
    return false;
  }

  if ((*session = calloc(1, sizeof(libcache_session_t))) == NULL) {
    return false;
  }

  if (args != NULL) {
    (*session)->performance_register_div64 = args->performance_register_div64;
  }

  (*session)->memory.pagemap = open("/proc/self/pagemap", O_RDONLY);
  if ((*session)->memory.pagemap == -1) {
    free(*session);
    return false;
  }

#if TIME_SOURCE == TIME_SOURCE_PERF
  perf_init(*session, args);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
  thread_counter_init(*session, args);
#endif

  libcache_eviction_init(*session, args);

#if defined(__ARM_ARCH_7A__)
  arm_v7_init(*session, args);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_init(*session, args);
#endif

  return true;
}

void
libcache_flush(libcache_session_t* session, void* address)
{
  (void) session;

#if USE_EVICTION == 1
  libcache_eviction_evict(session, address);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_flush(address);
#else
#error No flush/eviction method available on this platform
#endif

}

uint64_t
libcache_flush_time(libcache_session_t* session, void* address)
{
	(void) session;

	uint64_t start = libcache_get_timing(session);

#if USE_EVICTION == 1
  libcache_eviction_evict(session, address);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_flush(address);
#else
#error No flush/eviction method available on this platform
#endif

	uint64_t end = libcache_get_timing(session);
	
	return end - start;
}

void
libcache_evict(libcache_session_t* session, void* address)
{
  (void) session;

  libcache_eviction_evict(session, address);
}

uint64_t
libcache_evict_time(libcache_session_t* session, void* address)
{
	(void) session;

	uint64_t start = libcache_get_timing(session);

	libcache_eviction_evict(session, address);

	uint64_t end = libcache_get_timing(session);
	
	return end - start;
}

bool
libcache_terminate(libcache_session_t* session)
{
  (void) session;

  if (session == NULL) {
    return false;
  }

  if (session->memory.pagemap >= 0) {
    close(session->memory.pagemap);
  }
  session->memory.pagemap = -1;

#if TIME_SOURCE == TIME_SOURCE_PERF
  perf_terminate(session);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
  thread_counter_terminate(session);
#endif

  libcache_eviction_terminate(session);

#if defined(__ARM_ARCH_7A__)
  arm_v7_terminate(session);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_terminate(session);
#endif

  free(session);

  return true;
}

uint64_t
libcache_get_timing(libcache_session_t* session)
{
  (void) session;

  uint64_t result = 0;

  libcache_memory_barrier();

#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
  result = get_monotonic_time();
#elif TIME_SOURCE == TIME_SOURCE_PERF
  result = perf_get_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
  result = thread_counter_get_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_REGISTER
#if defined(__ARM_ARCH_7A__)
  result = arm_v7_get_timing();
#elif defined(__ARM_ARCH_8A__)
  result = arm_v8_get_timing();
#endif
#endif

  libcache_memory_barrier();

  return result;
}

uint64_t
libcache_get_timing_start(libcache_session_t* session)
{
  (void) session;

  uint64_t result = 0;

  result = libcache_get_timing(session);

  return result;
}

uint64_t
libcache_get_timing_end(libcache_session_t* session)
{
  (void) session;

  uint64_t result = 0;

  result = libcache_get_timing(session);

  return result;
}


inline void
libcache_access_memory(void *address) {

#if defined(__ARM_ARCH_7A__)
  arm_v7_access_memory(address);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_access_memory(address);
#endif

}


inline void
libcache_memory_barrier()
{

#if defined(__ARM_ARCH_7A__)
  arm_v7_memory_barrier();
#elif defined(__ARM_ARCH_8A__)
  arm_v8_memory_barrier();
#endif

}

void
libcache_prime(libcache_session_t* session, size_t set_index)
{

  libcache_eviction_prime(session, set_index);

}

size_t
libcache_get_set_index(libcache_session_t* session, void* address)
{

  return libcache_eviction_get_set_index(session, address);

}

size_t
libcache_get_number_of_sets(libcache_session_t* session)
{

  return libcache_eviction_get_number_of_sets(session);

}


uint64_t
libcache_probe(libcache_session_t* session, size_t set_index)
{

  return libcache_eviction_probe(session, set_index);

}

uint64_t
libcache_reload_address(libcache_session_t* session, void* address)
{
	uint64_t time = libcache_get_timing_start(session);
	libcache_access_memory(address);
	uint64_t delta = libcache_get_timing_end(session) - time;
	return delta;
}

uint64_t
libcache_get_basetime(libcache_session_t* session)
{
	uint64_t time = libcache_get_timing_start(session);
	uint64_t delta = libcache_get_timing_end(session) - time;
	return delta;
}

uint64_t
libcache_reload_address_and_evict(libcache_session_t* session, void* address)
{
	uint64_t time = libcache_get_timing_start(session);
	libcache_access_memory(address);
	uint64_t delta = libcache_get_timing_end(session) - time;
	libcache_evict(session, address);
	return delta;
}

uint64_t
libcache_reload_address_and_evict_other_address(libcache_session_t* session, void* address1, void* address2)
{
	uint64_t time = libcache_get_timing_start(session);
	libcache_access_memory(address1);
	uint64_t delta = libcache_get_timing_end(session) - time;
	libcache_evict(session, address2);
	return delta;
}

uintptr_t
libcache_get_physical_address(libcache_session_t* session, uintptr_t virtual_address)
{
  (void) session;
  (void) virtual_address;

  libcache_access_memory((void *) virtual_address);

  uint64_t value;
  off_t offset = (virtual_address / 4096) * sizeof(value);
  int got = pread(session->memory.pagemap, &value, sizeof(value), offset);
  assert(got == 8);

  assert(value & (1ULL << 63));

  uint64_t frame_num = get_frame_number_from_pagemap(value);
  return (frame_num * 4096) | (virtual_address & (4095));
}

uint64_t
libcache_get_pagemap_entry(libcache_session_t* session, uint64_t virtual_address)
{
  (void) session;
  (void) virtual_address;

  uint64_t value;
  off_t offset = (virtual_address / 4096) * sizeof(value);
  int got = pread(session->memory.pagemap, &value, sizeof(value), offset);
  assert(got == 8);

  return value;
}

static size_t
get_frame_number_from_pagemap(size_t value)
{
  return value & ((1ULL << 55) - 1);
}
