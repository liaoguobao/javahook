//HookTrampoline.cpp

#include <sys/mman.h>
#include <string.h>
#include "HookTrampoline.h"
#include "hook_trampoline_arm.h"

extern "C" void replacement_hook_trampoline();
extern "C" void __clear_cache(void *beg, void *end);

CHookTrampoline::CHookTrampoline()
{
    num = 0;
    code = (unsigned char *)mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, -1, 0);
}

CHookTrampoline::~CHookTrampoline()
{
    munmap(code, PAGE_SIZE);
}

CHookTrampoline * CHookTrampoline::GetInstance()
{
    static CHookTrampoline thiz;
    return &thiz;
}

const unsigned char * CHookTrampoline::CreateTrampoline(void *art_method, unsigned char offset_compiled_code)
{
    if(!art_method || !code || num>=MAX_REPLACEMENT_HOOK_TRAMPOLINE)
        return 0;

    mprotect(code, PAGE_SIZE, PROT_READ|PROT_EXEC|PROT_WRITE);

    unsigned char *codeaddr = code + num*SIZE_REPLACEMENT_HOOK_TRAMPOLINE;
    memcpy(codeaddr, (void *)replacement_hook_trampoline, SIZE_REPLACEMENT_HOOK_TRAMPOLINE);

    *(void **)&codeaddr[ADDRESS_REPLACEMENT_ART_METHOD] = art_method;
    *(unsigned char *)&codeaddr[ADDRESS_REPLACEMENT_COMPILED_CODE] = offset_compiled_code;

    mprotect(code, PAGE_SIZE, PROT_READ|PROT_EXEC);
    __clear_cache(code, code+PAGE_SIZE);

    num++;
    return codeaddr;
}
