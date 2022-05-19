//hook_trampoline_arm.h

#ifndef _HOOK_TRAMPOLINE_ARM_H_
#define _HOOK_TRAMPOLINE_ARM_H_

#define __ANDROID_API_VER__   Lollipop_5_0

#define __ANDROID_API_MIN__   Lollipop_5_0
#define __ANDROID_API_MAX__   AndroidS_12_0

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
#define __ANDROID_API_S__     31 //12.0

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
#define AndroidS_12_0         __ANDROID_API_S__

#define POINTER_SIZE          sizeof(void*)
#define POINTER_ROUND_DOWN(x) (((size_t)(x))                   & (~(POINTER_SIZE-1)))
#define POINTER_ROUND_UP(x)  ((((size_t)(x)) + POINTER_SIZE-1) & (~(POINTER_SIZE-1)))

#if __ANDROID_API_VER__ == Lollipop_5_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     56
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(24) //5.0版本没有ptr_sized_fields_字段
#define OFFSET_REPLACEMENT_INTERPRETER_CODE (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_REPLACEMENT_INTERPRETER_CODE+sizeof(uint64_t))
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+sizeof(uint64_t))
#elif __ANDROID_API_VER__ == Lollipop_5_1
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     20
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(36)
#define OFFSET_REPLACEMENT_INTERPRETER_CODE (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_REPLACEMENT_INTERPRETER_CODE+POINTER_SIZE)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == Marshmallow_6_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     12
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(28)
#define OFFSET_REPLACEMENT_INTERPRETER_CODE (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0) //6.0以后的版本去掉了entry_point_from_interpreter_字段,此宏也被移除
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_REPLACEMENT_INTERPRETER_CODE+POINTER_SIZE)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == Nougat_7_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*2)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == Nougat_7_1
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*2)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == Oreo_8_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*1)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == Oreo_8_1
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*1)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == Pie_9_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == AndroidQ_10_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == AndroidR_11_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(20)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#elif __ANDROID_API_VER__ == AndroidS_12_0
#define OFFSET_REPLACEMENT_ACCESS_FLAGS     4
#define OFFSET_PTR_SIZED_FIELDS             POINTER_ROUND_UP(16)
#define OFFSET_REPLACEMENT_JNI_CODE         (OFFSET_PTR_SIZED_FIELDS+POINTER_SIZE*0)
#define OFFSET_REPLACEMENT_COMPILED_CODE    (OFFSET_REPLACEMENT_JNI_CODE+POINTER_SIZE)
#endif

#define kAccPublic                          0x00000001 // class, field, method, ic
#define kAccPrivate                         0x00000002 // field, method, ic
#define kAccProtected                       0x00000004 // field, method, ic
#define kAccNative                          0x00000100 // method
#define kAccAbstract                        0x00000400 // class, method, ic
#define kAccTweakHookedMethod               0x00008000 // Set for a method that has been hooked by tweaker.
#define kAccFastInterpreterToInterpreterInvoke 0x40000000 // >=10.0 Non-intrinsics: Caches whether we can use fast-path in the interpreter invokes.
#if __ANDROID_API_VER__ >= Nougat_7_0 && __ANDROID_API_VER__ <= Oreo_8_0
#define kAccCompileDontBother               0x01000000 // Set by the verifier for a method we do not want the compiler to compile.(7.0 7.1 8.0)
#elif __ANDROID_API_VER__ >= Oreo_8_1
#define kAccCompileDontBother               0x02000000 // Set by the verifier for a method we do not want the compiler to compile.(>=8.1)
#endif
#define kAccPreviouslyWarm                  0x00800000 // >=8.1 Set by the JIT when clearing profiling infos to denote that a method was previously warm.

#if defined(__arm__)
#define HOOK_COMPILED_CODE_OFFSET       4
#define HOOK_ART_METHOD_ADDRESS         8
#elif defined(__aarch64__)
#define HOOK_COMPILED_CODE_OFFSET       5
#define HOOK_ART_METHOD_ADDRESS         12
#endif
#define ORIGIN_REPLACE_TRAMPOLINE_SIZE  (HOOK_ART_METHOD_ADDRESS+sizeof(void *))

#if defined(__arm__)
#define ORIG_ART_METHOD_ADDRESS         16
#elif defined(__aarch64__)
#define ORIG_ART_METHOD_ADDRESS         12
#endif
#define ORIG_COMPILED_CODE_ADDRESS      (ORIG_ART_METHOD_ADDRESS+sizeof(void *))
#define BACKUP_REPLACE_TRAMPOLINE_SIZE  (ORIG_COMPILED_CODE_ADDRESS+sizeof(void *))

#define HOOK_PAGE_SIZE                  0x1000
#define REPLACE_TRAMPOLINE_SIZE         (ORIGIN_REPLACE_TRAMPOLINE_SIZE+BACKUP_REPLACE_TRAMPOLINE_SIZE)
#define MAX_REPLACE_TRAMPOLINE_INDEX    (HOOK_PAGE_SIZE/REPLACE_TRAMPOLINE_SIZE) //113

#endif
