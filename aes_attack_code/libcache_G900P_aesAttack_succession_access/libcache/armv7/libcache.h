/* See LICENSE file for license and copyright information */

#ifndef ARM_V7_LIBCACHE_H
#define ARM_V7_LIBCACHE_H

#include "../libcache.h"

#include "timing.h"
#include "memory.h"

void arm_v7_init(libcache_session_t* session, libcache_session_args_t* args);
void arm_v7_terminate(libcache_session_t* session);

#endif /* ARM_V7_LIBFLUSH_H */
