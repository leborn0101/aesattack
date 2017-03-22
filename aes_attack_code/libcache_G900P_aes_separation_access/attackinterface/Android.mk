LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include ../config.mk
LOCAL_MODULE := libcache
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../
LOCAL_SRC_FILES := ../obj/local/$(TARGET_ARCH_ABI)/libcache.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS += ${CFLAGS}
LOCAL_MODULE := attack_interfaces
LOCAL_SRC_FILES := gaddr.h gset.h androidlog.h AES.h AES.c main.c
LOCAL_SHARED_LIBRARIES := libcache
LOCAL_LDLIBS := -lm -llog
include $(BUILD_SHARED_LIBRARY)
