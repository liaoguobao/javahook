//HookTrampoline.cpp

#include <sys/mman.h>
#include <string.h>
#include "HookTrampoline.h"
#include "hook_trampoline_arm.h"

extern "C" void replacement_origin_trampoline();
extern "C" void replacement_backup_trampoline();
extern "C" void __clear_cache(void *beg, void *end);

CHookTrampoline::CHookTrampoline()
{
    backup = 0;
    origin = 0;
    code = (unsigned char *)mmap(0, HOOK_PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, -1, 0);
}

CHookTrampoline::~CHookTrampoline()
{
    munmap(code, HOOK_PAGE_SIZE);
}

CHookTrampoline * CHookTrampoline::GetInstance()
{
    static CHookTrampoline thiz;
    return &thiz;
}

const unsigned char * CHookTrampoline::CreateOriginTrampoline(void *hook_art_method, unsigned char offset_compiled_code)
{
    if(!hook_art_method || !code || origin>=MAX_REPLACE_TRAMPOLINE_INDEX)
        return 0;

    mprotect(code, HOOK_PAGE_SIZE, PROT_READ|PROT_EXEC|PROT_WRITE);

    unsigned char *codeaddr = code + origin*REPLACE_TRAMPOLINE_SIZE;
    memcpy(codeaddr, (void *)replacement_origin_trampoline, ORIGIN_REPLACE_TRAMPOLINE_SIZE);

    *(void **)&codeaddr[HOOK_ART_METHOD_ADDRESS] = hook_art_method;
#if defined(__arm__)
    *(unsigned char *)&codeaddr[HOOK_COMPILED_CODE_OFFSET+0] |= offset_compiled_code;
#elif defined(__aarch64__)
    *(unsigned char *)&codeaddr[HOOK_COMPILED_CODE_OFFSET+0] |= offset_compiled_code << 4;
    *(unsigned char *)&codeaddr[HOOK_COMPILED_CODE_OFFSET+1] |= offset_compiled_code >> 4;
#endif
    mprotect(code, HOOK_PAGE_SIZE, PROT_READ|PROT_EXEC);
    __clear_cache(code, code+HOOK_PAGE_SIZE);

    origin++;
    return codeaddr;
}

const unsigned char * CHookTrampoline::CreateBackupTrampoline(void *orig_art_method, void *orig_compiled_code)
{
    if(!orig_art_method || !code || backup>=MAX_REPLACE_TRAMPOLINE_INDEX)
        return 0;

    mprotect(code, HOOK_PAGE_SIZE, PROT_READ|PROT_EXEC|PROT_WRITE);

    unsigned char *codeaddr = code + backup*REPLACE_TRAMPOLINE_SIZE + ORIGIN_REPLACE_TRAMPOLINE_SIZE;
    memcpy(codeaddr, (void *)replacement_backup_trampoline, BACKUP_REPLACE_TRAMPOLINE_SIZE);

    *(void **)&codeaddr[ORIG_ART_METHOD_ADDRESS] = orig_art_method;
    *(void **)&codeaddr[ORIG_COMPILED_CODE_ADDRESS] = orig_compiled_code;

    mprotect(code, HOOK_PAGE_SIZE, PROT_READ|PROT_EXEC);
    __clear_cache(code, code+HOOK_PAGE_SIZE);

    backup++;
    return codeaddr;
}
