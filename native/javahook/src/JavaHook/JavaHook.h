//JavaHook.h

#ifndef _JAVA_HOOK_H_
#define _JAVA_HOOK_H_

#include <jni.h>

class CJavaHook
{
protected:
    CJavaHook();
    ~CJavaHook();

public:
    static int InitJavaHook(JavaVM *vm = 0);

    static int LoadDexFile(JNIEnv *env, const char *dexfile, int opt = 1);
    static int OptDexFile(JNIEnv *env, const char *dexfile, const char *oatfile);

    static jobject BackupMethod(JNIEnv *env, jobject method);
    static int     ModifyMethod(JNIEnv *env, jobject method_orig, jobject method_back, jobject method_hook, int is_uninit = 0);

    static jobject FindClassMethod(JNIEnv *env, jclass clazz, const char *method);
    static jobject FindClassMethod(JNIEnv *env, const char *clazz, const char *method);

    static jobject HookMethod(JNIEnv *env, jclass tweak_class, const char *tweak_method, jclass hook_class, const char *hook_method);
    static jobject HookMethod(JNIEnv *env, const char *tweak_class, const char *tweak_method, const char *hook_class, const char *hook_method);
};

#endif
