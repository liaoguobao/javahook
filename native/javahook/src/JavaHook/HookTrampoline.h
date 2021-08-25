//HookTrampoline.h

#ifndef _HOOK_TRAMPOLINE_H_
#define _HOOK_TRAMPOLINE_H_

class CHookTrampoline
{
protected:
    unsigned int origin;
    unsigned int backup;
    unsigned char *code;

protected:
    CHookTrampoline();
    ~CHookTrampoline();

public:
    static CHookTrampoline * GetInstance();

public:
    const unsigned char * CreateOriginTrampoline(void *hook_art_method, unsigned char offset_compiled_code);
    const unsigned char * CreateBackupTrampoline(void *orig_art_method, void *orig_compiled_code);
};

#endif
