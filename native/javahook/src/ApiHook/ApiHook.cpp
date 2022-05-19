//ApiHook.cpp

#include "../include/platform.h"
#if defined(_WIN32)
#include "../include/detours/detours.h"
#elif defined(__arm__)
#include "../include/substrate/substrate.h"
#elif defined(__aarch64__)
#include "../include/funchook/funchook.h"
#endif
#if defined(__APPLE__)
#include "../ObjcUtil/ObjcUtil.h"
#endif
#include "ApiHook.h"

CApiHook::CApiHook()
{
}

CApiHook::~CApiHook()
{
}

int CApiHook::HookFunction(size_t symbol, size_t hook, size_t *old)
{
    if(symbol == 0 || hook == 0)
        return FALSE;

    if(old) *old = 0;
    size_t _old = symbol;
#if defined(_WIN32)
    ::DetourTransactionBegin();
    ::DetourUpdateThread(::GetCurrentThread());
    ::DetourAttach((void **)&_old, (void *)hook);
    ::DetourTransactionCommit();
#elif defined(__arm__)
    MSHookFunction((void *)symbol, (void *)hook, (void **)&_old);
#elif defined(__aarch64__)
    funchook_t *fh = funchook_create();
    funchook_prepare(fh, (void **)&_old, (void *)hook);
    funchook_install(fh, 0);
#endif
    if(old) *old = _old;
    return TRUE;
}

int CApiHook::HookFunction(void *handle, const char *name, size_t hook, size_t *old)
{
    if(handle == 0 || name == 0 || hook == 0)
        return FALSE;

    if(old) *old = 0;
#if defined(_WIN32)
    size_t symbol = (size_t)::GetProcAddress((HMODULE)handle, name);
#else
    size_t symbol = (size_t)dlsym(handle, name);
#endif
    if(!symbol)
        return FALSE;

    if(!HookFunction(symbol, hook, old))
        return FALSE;

    return TRUE;
}

int CApiHook::HookFunction(const char *module, const char *name, size_t hook, size_t *old)
{
    if(module == 0 || name == 0 || hook == 0)
        return FALSE;

    if(old) *old = 0;
#if defined(_WIN32)
    void *handle = (void *)::LoadLibraryExA(module, NULL, 0);
#else
    void *handle = (void *)dlopen(module, 0);
#endif
    if(!handle)
        return FALSE;

    if(!HookFunction(handle, name, hook, old))
        return FALSE;

    return TRUE;
}
#if defined(_WIN32)
int CApiHook::GetInterfaceAddress(void *iunknown, unsigned index, size_t *addr)
{
    if(!iunknown || !addr)
        return FALSE;

    *addr = *(*(size_t **)iunknown + index);
    return TRUE;
}
#elif defined(__APPLE__)
int CApiHook::HookMessage(const char *declare, size_t hook, size_t *old)//+[APDeviceColorManager updateColorLabel:label:]
{
    if(!declare || !*declare || !hook)
        return FALSE;

    char flag = *declare++;
    if((flag!='+' && flag!='-') || (*declare++ != '['))
        return FALSE;

    const char *pos = strchr(declare, ' ');
    if(!pos)
        return FALSE;

    string _class(declare, pos-declare);
    string _message(++pos);
    if(_message[_message.size()-1] != ']')
        return FALSE;

    Class cls = 0;
    if(flag == '+') //static
        cls = objc_getMetaClass(_class.c_str());
    else if(flag == '-') //instance
        cls = objc_getClass(_class.c_str());
    if(!cls)
        return FALSE;

    _message.resize(_message.size()-1);
    SEL sel = sel_registerName(_message.c_str());
    if(!sel)
        return FALSE;

    MSHookMessageEx(cls, sel, (void *)hook, (void **)old);
    return TRUE;
}
#endif
