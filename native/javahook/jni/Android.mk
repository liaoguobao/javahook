
LOCAL_PATH := $(call my-dir)

ifeq ($(APP_LDFLAGS), -shared)
ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
include $(CLEAR_VARS)
LOCAL_MODULE    := funchook
LOCAL_SRC_FILES := ../lib/funchook/libfunchook_$(TARGET_ARCH_ABI).a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE    := capstone
LOCAL_SRC_FILES := ../lib/funchook/libcapstone_$(TARGET_ARCH_ABI).a
include $(PREBUILT_STATIC_LIBRARY)
else ifeq ($(TARGET_ARCH_ABI), armeabi)
include $(CLEAR_VARS)
LOCAL_MODULE    := substrate
LOCAL_SRC_FILES := ../lib/substrate/libsubstrate_$(TARGET_ARCH_ABI).a
include $(PREBUILT_STATIC_LIBRARY)
endif
endif

include $(CLEAR_VARS)

TARGET_PLATFORM := android-24
LOCAL_MODULE := javahook

LOCAL_SRC_FILES := \
	../src/javatweak.cpp \
	../src/ApiHook/ApiHook.cpp \
	../src/JavaHook/JavaHook.cpp \
	../src/JavaHook/HookTrampoline.cpp \
	../src/JavaHook/HookHelper.cpp \

ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
LOCAL_SRC_FILES += \
	../src/JavaHook/asm_support_arm64.S \
	../src/JavaHook/hook_trampoline_arm64.S 
else ifeq ($(TARGET_ARCH_ABI), armeabi)
LOCAL_SRC_FILES += \
	../src/JavaHook/asm_support_arm.S \
	../src/JavaHook/hook_trampoline_arm.S 
endif

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../src/include \

ifeq ($(APP_LDFLAGS), -shared)
LOCAL_LDLIBS := -llog
ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
LOCAL_STATIC_LIBRARIES := funchook capstone
else ifeq ($(TARGET_ARCH_ABI), armeabi)
LOCAL_STATIC_LIBRARIES := substrate
endif
endif

ifeq ($(APP_LDFLAGS), -shared)
include $(BUILD_SHARED_LIBRARY)
else ifeq ($(APP_LDFLAGS), -static)
include $(BUILD_STATIC_LIBRARY)
else
include $(BUILD_EXECUTABLE)
endif
