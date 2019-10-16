#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := uimremoteclienttestapp
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_JAVA_LIBRARIES := uimremoteclientlibrary

LOCAL_SRC_FILES := $(call all-subdir-java-files)

include $(BUILD_PACKAGE)
