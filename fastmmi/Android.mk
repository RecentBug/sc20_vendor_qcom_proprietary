ifeq ($(call is-board-platform-in-list,msm8909 msm8916 msm8952 msm8996 msm8937 msm8953 msm8998),true)
include $(call all-subdir-makefiles)
endif
