LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SDK_VERSION := current
LOCAL_PACKAGE_NAME := LatamTelefonicaNicaraguaSystemUIRes
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/LatamTelefonicaNicaragua/system/vendor/overlay
LOCAL_CERTIFICATE := shared

include $(BUILD_PACKAGE)

