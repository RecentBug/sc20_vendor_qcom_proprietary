ar0542_CHROMATIX_SNAPSHOT_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library (libchromatix_ar0542_snapshot)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(ar0542_CHROMATIX_SNAPSHOT_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS:= -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../common/
LOCAL_C_INCLUDES += chromatix_ar0542_snapshot.h

LOCAL_SRC_FILES:= chromatix_ar0542_snapshot.c

LOCAL_MODULE           := libchromatix_ar0542_snapshot
LOCAL_SHARED_LIBRARIES := libcutils
include $(LOCAL_PATH)/../../../../../../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)