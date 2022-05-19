//asm_support_arm64.S

#ifndef _ASM_SUPPORT_ARM64_S_
#define _ASM_SUPPORT_ARM64_S_

#if defined(__aarch64__)

.macro FUNCTION_START_ARM64 name
    .type \name, #function
    .hidden \name  // Hide this as a global symbol, so we do not incur plt calls.
    .global \name
    /* Cache alignment for function entry */
    .balign 16
\name:
    .cfi_startproc
.endm

.macro FUNCTION_START_ARM64_NOHIDE name
    .type \name, #function
    .global \name
    /* Cache alignment for function entry */
    .balign 16
\name:
    .cfi_startproc
.endm

.macro FUNCTION_END name
    .cfi_endproc
    .size \name, .-\name
.endm

.macro UNIMPLEMENTED name
    FUNCTION_START_ARM64 \name
    brk 0
    FUNCTION_END \name
.endm

// Macros to poison (negate) the reference for heap poisoning.
.macro POISON_HEAP_REF rRef
#ifdef USE_HEAP_POISONING
    neg \rRef, \rRef
#endif  // USE_HEAP_POISONING
.endm

// Macros to unpoison (negate) the reference for heap poisoning.
.macro UNPOISON_HEAP_REF rRef
#ifdef USE_HEAP_POISONING
    neg \rRef, \rRef
#endif  // USE_HEAP_POISONING
.endm

#endif

#endif
