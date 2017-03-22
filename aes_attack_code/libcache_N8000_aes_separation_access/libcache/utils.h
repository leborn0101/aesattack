#ifndef _LIBCACHE_UTILS_H
#define _LIBCACHE_UTILS_H

#include <stdbool.h>
#include <unistd.h>
#include <android/log.h>

#define LOGTAG "myjni"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__)

bool libcache_bind_to_cpu(size_t cpu);


bool libcache_bind_aes_to_cpu(int pid, size_t cpu);


#endif
