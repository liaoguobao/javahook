//asm_support_arm.s

#ifndef _ASM_SUPPORT_ARM_S_
#define _ASM_SUPPORT_ARM_S_

#if defined(__arm__)

.syntax unified
.arch armv7-a
.thumb

.macro FUNCTION_START_THUMB name
    .thumb_func
    .type \name, #function
    .hidden \name  // Hide this as a global symbol, so we do not incur plt calls.
    .global \name
    /* Cache alignment for function entry */
    .balign 4
\name:
    .cfi_startproc
    .fnstart
.endm

.macro FUNCTION_START_THUMB_NOHIDE name
    .thumb_func
    .type \name, #function
    .global \name
    /* Cache alignment for function entry */
    .balign 4
\name:
    .cfi_startproc
    .fnstart
.endm


.macro FUNCTION_START_ARM name
    .arm
    .type \name, #function
    .hidden \name  // Hide this as a global symbol, so we do not incur plt calls.
    .global \name
    /* Cache alignment for function entry */
    .balign 4
\name:
    .cfi_startproc
     /* Ensure we get a sane starting CFA. */
    .cfi_def_cfa sp,0
    .fnstart
.endm

.macro FUNCTION_START_ARM_NOHIDE name
    .arm
    .type \name, #function
    .global \name
    /* Cache alignment for function entry */
    .balign 4
\name:
    .cfi_startproc
     /* Ensure we get a sane starting CFA. */
    .cfi_def_cfa sp,0
    .fnstart
.endm

.macro FUNCTION_END name
    .fnend
    .cfi_endproc
    .size \name, .-\name
.endm

#endif

#endif
