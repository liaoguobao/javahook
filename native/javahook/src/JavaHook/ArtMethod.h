//ArtMethod.h

#ifndef _ART_METHOD_H_
#define _ART_METHOD_H_

#include <stdint.h>
#include "hook_trampoline_arm.h"

#define PACKED(x) __attribute__((__aligned__(x), __packed__))

#if __ANDROID_API_VER__ == Lollipop_5_0
struct ArtMethod { //Lollipop 5.0.0_r2
    // The Class representing the type of the object.
    //HeapReference<Class> klass_;
    uint32_t klass_;
    // Monitor and hash code information.
    uint32_t monitor_;

#ifdef USE_BAKER_OR_BROOKS_READ_BARRIER
    // Note names use a 'x' prefix and the x_rb_ptr_ is of type int
    // instead of Object to go with the alphabetical/by-type field order
    // on the Java side.
    uint32_t x_rb_ptr_;      // For the Baker or Brooks pointer.
    uint32_t x_xpadding_;    // For 8-byte alignment. TODO: get rid of this.
#endif
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //HeapReference<Class> declaring_class_;
    uint32_t declaring_class_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //HeapReference<ObjectArray<ArtMethod>> dex_cache_resolved_methods_;
    uint32_t dex_cache_resolved_methods_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //HeapReference<ObjectArray<Class>> dex_cache_resolved_types_;
    uint32_t dex_cache_resolved_types_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //HeapReference<ObjectArray<String>> dex_cache_strings_;
    uint32_t dex_cache_strings_;

    // Method dispatch from the interpreter invokes this pointer which may cause a bridge into
    // compiled code.
    uint64_t entry_point_from_interpreter_;

    // Pointer to JNI function registered to this method, or a function to resolve the JNI function.
    uint64_t entry_point_from_jni_;

    // Method dispatch from portable compiled code invokes this pointer which may cause bridging into
    // quick compiled code or the interpreter.
#if defined(ART_USE_PORTABLE_COMPILER)
    uint64_t entry_point_from_portable_compiled_code_;
#endif

    // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
    // portable compiled code or the interpreter.
    uint64_t entry_point_from_quick_compiled_code_;

    // Pointer to a data structure created by the compiler and used by the garbage collector to
    // determine which registers hold live references to objects within the heap. Keyed by native PC
    // offsets for the quick compiler and dex PCs for the portable.
    uint64_t gc_map_;

    // Access flags; low 16 bits are defined by spec.
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint32_t method_index_;
};
#elif __ANDROID_API_VER__ == Lollipop_5_1
struct ArtMethod { //Lollipop 5.1.1_r6
    // The Class representing the type of the object.
    //HeapReference<Class> klass_;
    uint32_t klass_;
    // Monitor and hash code information.
    uint32_t monitor_;

#ifdef USE_BAKER_OR_BROOKS_READ_BARRIER
    // Note names use a 'x' prefix and the x_rb_ptr_ is of type int
    // instead of Object to go with the alphabetical/by-type field order
    // on the Java side.
    uint32_t x_rb_ptr_;      // For the Baker or Brooks pointer.
    uint32_t x_xpadding_;    // For 8-byte alignment. TODO: get rid of this.
#endif
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //HeapReference<Class> declaring_class_;
    uint32_t declaring_class_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //HeapReference<ObjectArray<ArtMethod>> dex_cache_resolved_methods_;
    uint32_t dex_cache_resolved_methods_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //HeapReference<ObjectArray<Class>> dex_cache_resolved_types_;
    uint32_t dex_cache_resolved_types_;

    // Access flags; low 16 bits are defined by spec.
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint32_t method_index_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PACKED(4) PtrSizedFields {
        // Method dispatch from the interpreter invokes this pointer which may cause a bridge into
        // compiled code.
        void* entry_point_from_interpreter_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function.
        void* entry_point_from_jni_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // portable compiled code or the interpreter.
        void* entry_point_from_quick_compiled_code_;

        // Method dispatch from portable compiled code invokes this pointer which may cause bridging
        // into quick compiled code or the interpreter. Last to simplify entrypoint logic.
#if defined(ART_USE_PORTABLE_COMPILER)
        void* entry_point_from_portable_compiled_code_;
#endif
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == Marshmallow_6_0
struct ArtMethod { //Marshmallow 6.0.1_r10
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //GcRoot<mirror::PointerArray> dex_cache_resolved_methods_;
    uint32_t dex_cache_resolved_methods_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    //GcRoot<mirror::ObjectArray<mirror::Class>> dex_cache_resolved_types_;
    uint32_t dex_cache_resolved_types_;

    // Access flags; low 16 bits are defined by spec.
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint32_t method_index_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    // PACKED(4) is necessary for the correctness of
    // RoundUp(OFFSETOF_MEMBER(ArtMethod, ptr_sized_fields_), pointer_size).
    struct PACKED(4) PtrSizedFields {
        // Method dispatch from the interpreter invokes this pointer which may cause a bridge into
        // compiled code.
        void* entry_point_from_interpreter_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function.
        void* entry_point_from_jni_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == Nougat_7_0
struct ArtMethod { //Nougat 7.0.0_r1
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    // The hotness we measure for this method. Managed by the interpreter. Not atomic, as we allow
    // missing increments: if the method is hot, we will see it eventually.
    uint16_t hotness_count_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    // PACKED(4) is necessary for the correctness of
    // RoundUp(OFFSETOF_MEMBER(ArtMethod, ptr_sized_fields_), pointer_size).
    struct PACKED(4) PtrSizedFields {
        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        //ArtMethod** dex_cache_resolved_methods_;
        void** dex_cache_resolved_methods_;

        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        //GcRoot<mirror::Class>* dex_cache_resolved_types_;
        void* dex_cache_resolved_types_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function,
        // or the profiling data for non-native methods, or an ImtConflictTable.
        void* entry_point_from_jni_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == Nougat_7_1
struct ArtMethod { //Nougat 7.1.2_r36
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    // The hotness we measure for this method. Managed by the interpreter. Not atomic, as we allow
    // missing increments: if the method is hot, we will see it eventually.
    uint16_t hotness_count_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    // PACKED(4) is necessary for the correctness of
    // RoundUp(OFFSETOF_MEMBER(ArtMethod, ptr_sized_fields_), pointer_size).
    struct PACKED(4) PtrSizedFields {
        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        //ArtMethod** dex_cache_resolved_methods_;
        void** dex_cache_resolved_methods_;

        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        //GcRoot<mirror::Class>* dex_cache_resolved_types_;
        void* dex_cache_resolved_types_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function,
        // or the profiling data for non-native methods, or an ImtConflictTable.
        void* entry_point_from_jni_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == Oreo_8_0
struct ArtMethod { //Oreo 8.0.0_r4
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    // The hotness we measure for this method. Managed by the interpreter. Not atomic, as we allow
    // missing increments: if the method is hot, we will see it eventually.
    uint16_t hotness_count_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        //ArtMethod** dex_cache_resolved_methods_;
        void** dex_cache_resolved_methods_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function,
        // or the profiling data for non-native methods, or an ImtConflictTable, or the
        // single-implementation of an abstract/interface method.
        void* data_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == Oreo_8_1
struct ArtMethod { //Oreo 8.1.0_r33
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    // The hotness we measure for this method. Managed by the interpreter. Not atomic, as we allow
    // missing increments: if the method is hot, we will see it eventually.
    uint16_t hotness_count_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
        // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
        //mirror::MethodDexCacheType* dex_cache_resolved_methods_;
        void* dex_cache_resolved_methods_;

        // Pointer to JNI function registered to this method, or a function to resolve the JNI function,
        // or the profiling data for non-native methods, or an ImtConflictTable, or the
        // single-implementation of an abstract/interface method.
        void* data_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == Pie_9_0
struct ArtMethod { //Pie 9.0.0_r3
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    // The hotness we measure for this method. Not atomic, as we allow
    // missing increments: if the method is hot, we will see it eventually.
    uint16_t hotness_count_;

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
        // Depending on the method type, the data is
        //   - native method: pointer to the JNI function registered to this method
        //                    or a function to resolve the JNI function,
        //   - conflict method: ImtConflictTable,
        //   - abstract/interface method: the single-implementation if any,
        //   - proxy method: the original interface method or constructor,
        //   - other methods: the profiling data.
        void* data_;

        // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
        // the interpreter.
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == AndroidQ_10_0
struct ArtMethod { //AndroidQ 10.0.0_r47
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    union {
      // Non-abstract methods: The hotness we measure for this method. Not atomic,
      // as we allow missing increments: if the method is hot, we will see it eventually.
      uint16_t hotness_count_;
      // Abstract methods: IMT index (bitwise negated) or zero if it was not cached.
      // The negation is needed to distinguish zero index and missing cached entry.
      uint16_t imt_index_;
    };

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
      // Depending on the method type, the data is
      //   - native method: pointer to the JNI function registered to this method
      //                    or a function to resolve the JNI function,
      //   - conflict method: ImtConflictTable,
      //   - abstract/interface method: the single-implementation if any,
      //   - proxy method: the original interface method or constructor,
      //   - other methods: the profiling data.
      void* data_;

      // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
      // the interpreter.
      void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == AndroidR_11_0
struct ArtMethod { //AndroidR 11.0.0_r21
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    union {
      // Non-abstract methods: The hotness we measure for this method. Not atomic,
      // as we allow missing increments: if the method is hot, we will see it eventually.
      uint16_t hotness_count_;
      // Abstract methods: IMT index (bitwise negated) or zero if it was not cached.
      // The negation is needed to distinguish zero index and missing cached entry.
      uint16_t imt_index_;
    };

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
      // Depending on the method type, the data is
      //   - native method: pointer to the JNI function registered to this method
      //                    or a function to resolve the JNI function,
      //   - conflict method: ImtConflictTable,
      //   - abstract/interface method: the single-implementation if any,
      //   - proxy method: the original interface method or constructor,
      //   - other methods: the profiling data.
      void* data_;

      // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
      // the interpreter.
      void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == AndroidS_12_0
struct ArtMethod { //AndroidS 12.0.0_r3
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    union {
      // Non-abstract methods: The hotness we measure for this method. Not atomic,
      // as we allow missing increments: if the method is hot, we will see it eventually.
      uint16_t hotness_count_;
      // Abstract methods: IMT index.
      uint16_t imt_index_;
    };

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
      // Depending on the method type, the data is
      //   - native method: pointer to the JNI function registered to this method
      //                    or a function to resolve the JNI function,
      //   - resolution method: pointer to a function to resolve the method and
      //                        the JNI function for @CriticalNative.
      //   - conflict method: ImtConflictTable,
      //   - abstract/interface method: the single-implementation if any,
      //   - proxy method: the original interface method or constructor,
      //   - other methods: during AOT the code item offset, at runtime a pointer
      //                    to the code item.
      void* data_;

      // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
      // the interpreter.
      void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#elif __ANDROID_API_VER__ == AndroidT_13_0
struct ArtMethod { //AndroidT 13.0.0_r3
    // Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
    // The class we are a part of.
    //GcRoot<mirror::Class> declaring_class_;
    uint32_t declaring_class_;

    // Access flags; low 16 bits are defined by spec.
    // Getting and setting this flag needs to be atomic when concurrency is
    // possible, e.g. after this method's class is linked. Such as when setting
    // verifier flags and single-implementation flag.
    //std::atomic<std::uint32_t> access_flags_;
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;

    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods the index is into
    // the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
    // ifTable.
    uint16_t method_index_;

    union {
      // Non-abstract methods: The hotness we measure for this method. Not atomic,
      // as we allow missing increments: if the method is hot, we will see it eventually.
      uint16_t hotness_count_;
      // Abstract methods: IMT index.
      uint16_t imt_index_;
    };

    // Fake padding field gets inserted here.

    // Must be the last fields in the method.
    struct PtrSizedFields {
      // Depending on the method type, the data is
      //   - native method: pointer to the JNI function registered to this method
      //                    or a function to resolve the JNI function,
      //   - resolution method: pointer to a function to resolve the method and
      //                        the JNI function for @CriticalNative.
      //   - conflict method: ImtConflictTable,
      //   - abstract/interface method: the single-implementation if any,
      //   - proxy method: the original interface method or constructor,
      //   - other methods: during AOT the code item offset, at runtime a pointer
      //                    to the code item.
      void* data_;

      // Method dispatch from quick compiled code invokes this pointer which may cause bridging into
      // the interpreter.
      void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;
};
#endif

class CArtMethod
{
protected:
    CArtMethod();
    ~CArtMethod();

public:
    static int ToString(const ArtMethod *am, string &s)
    {
        s.clear();
        char buf[256];

        sprintf(
        buf,
        "ArtMethod-ver(%d)-len(%d)-ptr(%p)->{\r\n",
        __ANDROID_API_VER__, (int)sizeof(ArtMethod), am);
        s += buf;
#if __ANDROID_API_VER__ >= Lollipop_5_0 && __ANDROID_API_VER__ <= Lollipop_5_1
        sprintf(
        buf,
        "\tklass_ = 0x%08x\r\n"
        "\tmonitor_ = 0x%08x\r\n",
        am->klass_,
        am->monitor_);
        s += buf;
#endif
#if __ANDROID_API_VER__ == Lollipop_5_0
        sprintf(
        buf,
        "\tdex_cache_strings_ = 0x%08x\r\n"
        "\tgc_map_ = %p\r\n",
        am->dex_cache_strings_,
        (void *)am->gc_map_);
        s += buf;
#endif
        sprintf(
        buf,
        "\tdeclaring_class_ = 0x%08x\r\n"
        "\taccess_flags_ = 0x%08x\r\n"
        "\tdex_method_index_ = 0x%08x\r\n"
        "\tmethod_index_ = 0x%04x\r\n",
        am->declaring_class_,
        am->access_flags_,
        am->dex_method_index_,
        am->method_index_);
        s += buf;
#if __ANDROID_API_VER__ <= AndroidR_11_0
        sprintf(
        buf,
        "\tdex_code_item_offset_ = 0x%08x\r\n",
        am->dex_code_item_offset_);
        s += buf;
#endif
#if __ANDROID_API_VER__ >= Nougat_7_0
        sprintf(
        buf,
        "\thotness_count_ = 0x%04x\r\n",
        am->hotness_count_);
        s += buf;
#endif
#if __ANDROID_API_VER__ >= Oreo_8_0
        sprintf(
        buf,
        "\tdata_ = %p\r\n",
        am->ptr_sized_fields_.data_);
        s += buf;
#endif
#if __ANDROID_API_VER__ <= Oreo_8_1
#if __ANDROID_API_VER__ >= Nougat_7_0
#define dex_cache_resolved_methods ptr_sized_fields_.dex_cache_resolved_methods_
#else
#define dex_cache_resolved_methods dex_cache_resolved_methods_
#endif
        sprintf(
        buf,
        "\tdex_cache_resolved_methods_ = %p\r\n",
        (void *)(size_t)am->dex_cache_resolved_methods);
        s += buf;
#endif
#if __ANDROID_API_VER__ <= Nougat_7_1
#if __ANDROID_API_VER__ >= Nougat_7_0
#define dex_cache_resolved_types ptr_sized_fields_.dex_cache_resolved_types_
#else
#define dex_cache_resolved_types dex_cache_resolved_types_
#endif
        sprintf(
        buf,
        "\tdex_cache_resolved_types_ = %p\r\n",
        (void *)(size_t)am->dex_cache_resolved_types);
        s += buf;
#endif
#if __ANDROID_API_VER__ <= Marshmallow_6_0
#if __ANDROID_API_VER__ >= Lollipop_5_1
#define entry_point_from_interpreter ptr_sized_fields_.entry_point_from_interpreter_
#else
#define entry_point_from_interpreter entry_point_from_interpreter_
#endif
        sprintf(
        buf,
        "\tentry_point_from_interpreter_ = %p\r\n",
        (void *)am->entry_point_from_interpreter);
        s += buf;
#endif
#if __ANDROID_API_VER__ <= Nougat_7_1
#if __ANDROID_API_VER__ >= Lollipop_5_1
#define entry_point_from_jni ptr_sized_fields_.entry_point_from_jni_
#else
#define entry_point_from_jni entry_point_from_jni_
#endif
        sprintf(
        buf,
        "\tentry_point_from_jni_ = %p\r\n",
        (void *)am->entry_point_from_jni);
        s += buf;
#endif
#if __ANDROID_API_VER__ >= Lollipop_5_1
#define entry_point_from_quick_compiled_code ptr_sized_fields_.entry_point_from_quick_compiled_code_
#else
#define entry_point_from_quick_compiled_code entry_point_from_quick_compiled_code_
#endif
        sprintf(
        buf,
        "\tentry_point_from_quick_compiled_code_ = %p\r\n",
        (void *)am->entry_point_from_quick_compiled_code);
        s += buf;
        s += "}\r\n";
        return 0;
    }
};

#endif
