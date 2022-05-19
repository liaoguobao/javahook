//hook_trampoline_arm.s

#ifndef _HOOK_TRAMPOLINE_ARM_S_
#define _HOOK_TRAMPOLINE_ARM_S_

#include "asm_support_arm.S"
#include "hook_trampoline_arm.h"

#if defined(__arm__)

FUNCTION_START_ARM replacement_origin_trampoline
    ldr r0, addr_hook_method    //00 00 9f e5
    ldr pc, [r0, #0x00]         //00 f0 90 e5
addr_hook_method:
    .long 0xffffffff
FUNCTION_END replacement_origin_trampoline

FUNCTION_START_ARM replacement_backup_trampoline
    ldr   r0, addr_orig_compiled//0c 00 9f e5
    push {r0}                   //04 00 2d e5
    ldr   r0, addr_orig_method  //00 00 9f e5
    pop  {pc}                   //04 f0 9d e4
addr_orig_method:
    .long 0xffffffff
addr_orig_compiled:
    .long 0xffffffff
FUNCTION_END replacement_backup_trampoline

#endif

#endif
