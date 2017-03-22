/* See LICENSE file for license and copyright information */

#include "libcache.h"
#include "internal.h"
#include "configuration.h"
#include "../timing.h"

void
arm_v7_init(libcache_session_t* session, libcache_session_args_t* args)
{
  (void) session;
  (void) args;

#if TIME_SOURCE == TIME_SOURCE_REGISTER
  bool div64 = CYCLE_COUNTER_DIV_64;
  if (args != NULL && args->performance_register_div64 == true) {
    div64 = true;
  }

  // Enable user space performance counter
  arm_v7_timing_init(div64);
#endif
}

void
arm_v7_terminate(libcache_session_t* session)
{
  (void) session;

  // Disable user space performance counter
#if TIME_SOURCE == TIME_SOURCE_REGISTER
  arm_v7_timing_terminate();
#endif
}
