//hook_trampoline_arm.h

#ifndef _HOOK_TRAMPOLINE_ARM_H_
#define _HOOK_TRAMPOLINE_ARM_H_

#define __ANDROID_API_VER__   KitKat_4_4

#define __ANDROID_API_MIN__   KitKat_4_4
#define __ANDROID_API_MAX__   AndroidR_11_0

#define __ANDROID_API_K__     19 //4.4
#define __ANDROID_API_K_MR1__ 20 //4.4W
#define __ANDROID_API_L__     21 //5.0
#define __ANDROID_API_L_MR1__ 22 //5.1
#define __ANDROID_API_M__     23 //6.0
#define __ANDROID_API_N__     24 //7.0
#define __ANDROID_API_N_MR1__ 25 //7.1
#define __ANDROID_API_O__     26 //8.0
#define __ANDROID_API_O_MR1__ 27 //8.1
#define __ANDROID_API_P__     28 //9.0
#define __ANDROID_API_Q__     29 //10.0
#define __ANDROID_API_R__     30 //11.0

#define KitKat_4_4            __ANDROID_API_K__
#define KitKat_4_4W           __ANDROID_API_K_MR1__
#define Lollipop_5_0          __ANDROID_API_L__
#define Lollipop_5_1          __ANDROID_API_L_MR1__
#define Marshmallow_6_0       __ANDROID_API_M__
#define Nougat_7_0            __ANDROID_API_N__
#define Nougat_7_1            __ANDROID_API_N_MR1__
#define Oreo_8_0              __ANDROID_API_O__
#define Oreo_8_1              __ANDROID_API_O_MR1__
#define Pie_9_0               __ANDROID_API_P__
#define AndroidQ_10_0         __ANDROID_API_Q__
#define AndroidR_11_0         __ANDROID_API_R__

#if __ANDROID_API_VER__ == Lollipop_5_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     56
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 24
#define OFFSET_REPLACEMENT_JNI_CODE         32
#define OFFSET_REPLACEMENT_COMPILED_CODE    40
#elif __ANDROID_API_VER__ == Lollipop_5_1
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     20
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 36
#define OFFSET_REPLACEMENT_JNI_CODE         40
#define OFFSET_REPLACEMENT_COMPILED_CODE    44
#elif __ANDROID_API_VER__ == Marshmallow_6_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     12
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 28
#define OFFSET_REPLACEMENT_JNI_CODE         32
#define OFFSET_REPLACEMENT_COMPILED_CODE    36
#elif __ANDROID_API_VER__ == Nougat_7_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 0 //7.0及以后的版本去掉了entry_point_from_interpreter_字段
#define OFFSET_REPLACEMENT_JNI_CODE         28
#define OFFSET_REPLACEMENT_COMPILED_CODE    32
#elif __ANDROID_API_VER__ == Nougat_7_1
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 0 //7.0及以后的版本去掉了entry_point_from_interpreter_字段
#define OFFSET_REPLACEMENT_JNI_CODE         28
#define OFFSET_REPLACEMENT_COMPILED_CODE    32
#elif __ANDROID_API_VER__ == Oreo_8_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 0 //7.0及以后的版本去掉了entry_point_from_interpreter_字段
#define OFFSET_REPLACEMENT_JNI_CODE         24
#define OFFSET_REPLACEMENT_COMPILED_CODE    28
#elif __ANDROID_API_VER__ == Oreo_8_1
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 0 //7.0及以后的版本去掉了entry_point_from_interpreter_字段
#define OFFSET_REPLACEMENT_JNI_CODE         24
#define OFFSET_REPLACEMENT_COMPILED_CODE    28
#elif __ANDROID_API_VER__ == Pie_9_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_REPLACEMENT_INTERPRETER_CODE 0 //7.0及以后的版本去掉了entry_point_from_interpreter_字段
#define OFFSET_REPLACEMENT_JNI_CODE         20
#define OFFSET_REPLACEMENT_COMPILED_CODE    24
#endif

#define kAccPublic                          0x00000001 // class, field, method, ic
#define kAccPrivate                         0x00000002 // field, method, ic
#define kAccProtected                       0x00000004 // field, method, ic
#define kAccNative                          0x00000100 // method
#define kAccTweakHookedMethod               0x00008000 // Set for a method that has been hooked by tweaker.
#define kAccFastInterpreterToInterpreterInvoke 0x40000000 //10.0
#if __ANDROID_API_VER__ >= Nougat_7_0 && __ANDROID_API_VER__ <= Oreo_8_0
#define kAccCompileDontBother               0x01000000 // Set by the verifier for a method we do not want the compiler to compile.(7.0 7.1 8.0)
#elif __ANDROID_API_VER__ >= Oreo_8_1
#define kAccCompileDontBother               0x02000000 // Set by the verifier for a method we do not want the compiler to compile.(>=8.1)
#endif

#define ADDRESS_REPLACEMENT_COMPILED_CODE   4
#define ADDRESS_REPLACEMENT_ART_METHOD      8
#define SIZE_REPLACEMENT_HOOK_TRAMPOLINE    12
#define MAX_REPLACEMENT_HOOK_TRAMPOLINE     (PAGE_SIZE/SIZE_REPLACEMENT_HOOK_TRAMPOLINE) //341

#endif
