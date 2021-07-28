//hook_trampoline_arm.s

#ifndef _HOOK_TRAMPOLINE_ARM_S_
#define _HOOK_TRAMPOLINE_ARM_S_

#include "asm_support_arm.S"
#include "hook_trampoline_arm.h"

#if defined(__arm__)

FUNCTION_START_ARM replacement_hook_trampoline
    ldr r0, addr_art_method //00 00 9f e5
    ldr pc, [r0, #0xff]     //ff f0 90 e5
addr_art_method:
    .long 0xffffffff
FUNCTION_END replacement_hook_trampoline

#endif

#endif
