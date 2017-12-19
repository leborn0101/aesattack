#ifndef LOG_H
#define LOG_H

#include <android/log.h>
#define LOG_TAG "my-jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG, __VA_ARGS__)

#endif
