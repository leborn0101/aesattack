/* See LICENSE file for license and copyright information */

#include "libcache.h"
#include "internal.h"
#include "../timing.h"

void
arm_v8_init(libcache_session_t* session, libcache_session_args_t* args)
{
  (void) session;
  (void) args;

  // Enable user space performance counter
#if TIME_SOURCE == TIME_SOURCE_REGISTER
  arm_v8_timing_init();
#endif
}

void
arm_v8_terminate(libcache_session_t* session)
{
  (void) session;

  // Disable user space performance counter
#if TIME_SOURCE == TIME_SOURCE_REGISTER
  arm_v8_timing_terminate();
#endif
}
