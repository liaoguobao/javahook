//hook_trampoline_arm64.S

#ifndef _HOOK_TRAMPOLINE_ARM64_S_
#define _HOOK_TRAMPOLINE_ARM64_S_

#include "asm_support_arm64.S"
#include "hook_trampoline_arm.h"

#if defined(__aarch64__)

FUNCTION_START_ARM64 replacement_origin_trampoline
    ldr  x0,  addr_hook_method   //60 00 00 58
    ldur x17, [x0, #0x00]        //11 00 40 f8
    br   x17                     //20 02 1f d6
addr_hook_method:
    .long 0xffffffff
    .long 0xffffffff
FUNCTION_END replacement_origin_trampoline

FUNCTION_START_ARM64 replacement_backup_trampoline
    ldr  x0,  addr_orig_method   //60 00 00 58
    ldr  x17, addr_orig_compiled //91 00 00 58
    br   x17                     //20 02 1f d6
addr_orig_method:
    .long 0xffffffff
    .long 0xffffffff
addr_orig_compiled:
    .long 0xffffffff
    .long 0xffffffff
FUNCTION_END replacement_backup_trampoline

#endif

#endif
