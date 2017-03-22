//
// Created by f on 2016/11/9.
//

#ifndef DPLSTEE_TEST_AES_ANDROIDLOG_H
#define DPLSTEE_TEST_AES_ANDROIDLOG_H

#include <android/log.h>
#define __DEBUG__
#define LOG_TAG "uTClient"
#define ALOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG,  __VA_ARGS__)

#define BOLI_LOG_TAG "boli_log"
#define BOLI_LOGI(...)  __android_log_print(ANDROID_LOG_INFO, BOLI_LOG_TAG,  __VA_ARGS__)

#define LOG_TAG "boli_log"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG,  __VA_ARGS__)

#endif //DPLSTEE_TEST_AES_ANDROIDLOG_H
