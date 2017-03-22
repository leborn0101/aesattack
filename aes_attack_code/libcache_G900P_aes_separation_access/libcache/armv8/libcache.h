/* See LICENSE file for license and copyright information */

#ifndef ARM_V8_LIBCACHE_H
#define ARM_V8_LIBCACHE_H

#include "../libcache.h"

#include "flush.h"
#include "timing.h"
#include "memory.h"

void arm_v8_init(libcache_session_t* session, libcache_session_args_t* args);
void arm_v8_terminate(libcache_session_t* session);

#endif /* ARM_V8_LIBCACHE_H */
