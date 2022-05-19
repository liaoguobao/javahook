//ApiHook.h

#ifndef _API_HOOK_H_
#define _API_HOOK_H_

class CApiHook
{
protected:
    CApiHook();
    ~CApiHook();

public:
    static int HookFunction(size_t symbol, size_t hook, size_t *old);
    static int HookFunction(void *handle, const char *name, size_t hook, size_t *old);
    static int HookFunction(const char *module, const char *name, size_t hook, size_t *old);
#if defined(_WIN32)
    static int GetInterfaceAddress(void *iunknown, unsigned index, size_t *addr);
#elif defined(__APPLE__)
    static int HookMessage(const char *declare, size_t hook, size_t *old);
#endif
};

#define EXTERN_C_API_HOOK_FUNCTION(name) \
{ \
    CApiHook::HookFunction((size_t)::name, (size_t)new_##name, (size_t *)&old_##name); \
}

#define EXTERN_INTERFACE_API_HOOK_FUNCTION(unk, index, name) \
{ \
    size_t addr = 0; \
    CApiHook::GetInterfaceAddress(unk, index, &addr); \
    CApiHook::HookFunction(addr, (size_t)new_##name, (size_t *)&old_##name); \
}

#define EXTERN_C_API_HOOK_FUNCTION_A(name) \
{ \
    EXTERN_C_API_HOOK_FUNCTION(name##A); \
}

#define EXTERN_C_API_HOOK_FUNCTION_W(name) \
{ \
    EXTERN_C_API_HOOK_FUNCTION(name##W); \
}

#define EXTERN_C_API_HOOK_FUNCTION_AW(name) \
{ \
    EXTERN_C_API_HOOK_FUNCTION_A(##name); \
    EXTERN_C_API_HOOK_FUNCTION_W(##name); \
}

#define EXTERN_JNI_API_HOOK_FUNCTION(_methods, _num, _name, _signature) \
for(int n = 0; n<_num && !old_JNI_##_name; ++n) \
{ \
    JNINativeMethod *nm = (JNINativeMethod *)_methods + n; \
    if(!strcmp(nm->name, #_name) && !strcmp(nm->signature, _signature)) \
    { \
        *(void **)&old_JNI_##_name = nm->fnPtr; \
        nm->fnPtr = (void *)new_JNI_##_name; \
    } \
}

#endif
