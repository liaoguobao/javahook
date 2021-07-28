
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := substrate
LOCAL_SRC_FILES := ../lib/substrate/libsubstrate_$(APP_ABI).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := javahook

LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_SRC_FILES := \
	../src/javatweak.cpp \
	../src/JavaHook/asm_support_arm.S \
	../src/JavaHook/hook_trampoline_arm.S \
	../src/JavaHook/HookTrampoline.cpp \
	../src/JavaHook/JavaHook.cpp \

LOCAL_C_INCLUDES := 

LOCAL_LDLIBS := \
	-llog \

LOCAL_STATIC_LIBRARIES := \
	substrate \

include $(BUILD_SHARED_LIBRARY)
