//HookTrampoline.h

#ifndef _HOOK_TRAMPOLINE_H_
#define _HOOK_TRAMPOLINE_H_

class CHookTrampoline
{
protected:
    unsigned int num;
    unsigned char *code;

protected:
    CHookTrampoline();
    ~CHookTrampoline();

public:
    static CHookTrampoline * GetInstance();

public:
    const unsigned char * CreateTrampoline(void *art_method, unsigned char offset_compiled_code);
};

#endif
