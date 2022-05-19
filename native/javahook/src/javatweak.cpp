//javahook.cpp

#include "include/JniUtil.h"
#include "include/ThreadLock.h"
#include "include/LinkerUtil.h"
#include "include/DebugUtil.h"
#include "JavaHook/JavaHook.h"
#include "ApiHook/ApiHook.h"

#define JAVAHOOK_PREFIX     "com/android/guobao/liao/apptweak/"
#define JAVAHOOK_DEXFILE    "javatweak.dex"
#define JAVAHOOK_DEXDIR     "/sdcard/tweak/"
#define JAVAHOOK_PACKAGE    "com.android.guobao.liao.apptweak."
#define JAVAHOOK_BRIDGE     "JavaTweakBridge"
#define JAVAHOOK_CALLBACK   "JavaTweakCallback"

static JavaVM          *g_vm = 0;
static jmethodID        g_defineJavaClass = 0;
static jclass           g_JavaTweakCallback = 0;
static int              g_sdkver = 0;
static int              g_initializing = 0;
static jobject        (*JNIEnvExt_NewLocalRef)(void *thiz, void* mirror) = 0;
static CThreadLock      g_lock;
static string           g_dexfile;
static map<size_t, int> g_initclass;

static void    JNI_printlog(JNIEnv *env, jclass clazz, int bufid, int prio, jstring tag, jstring msg);
static jobject JNI_hookmethod(JNIEnv *env, jclass clazz, jclass tweak_class, jstring tweak_method, jclass hook_class, jstring hook_method);

extern "C" JNIEXPORT int JNI_OnLoad(JavaVM *vm, void *reserved);

static int __javahook_initbridge__();
static int __javahook_defineclass__(const char *descriptor, void *class_loader, void *new_class);
static int __javahook_initializeclass__(void *klass);

//[==5.0]mirror::Class* ClassLinker::DefineClass(              const char* descriptor,              Handle<mirror::ClassLoader> class_loader, const DexFile& dex_file, const DexFile::ClassDef& dex_class_def)
static void *  new_ART_ClassLinker_DefineClass_5_0 (void *_this,             const char *descriptor,              void *class_loader, void *dex_file, void *dex_class_def);
static void *(*old_ART_ClassLinker_DefineClass_5_0)(void *_this,             const char *descriptor,              void *class_loader, void *dex_file, void *dex_class_def) = 0;

//[>=5.1]mirror::Class* ClassLinker::DefineClass(Thread* self, const char* descriptor, size_t hash, Handle<mirror::ClassLoader> class_loader, const DexFile& dex_file, const DexFile::ClassDef& dex_class_def)
static void *  new_ART_ClassLinker_DefineClass     (void *_this, void *self, const char *descriptor, size_t hash, void *class_loader, void *dex_file, void *dex_class_def);
static void *(*old_ART_ClassLinker_DefineClass)    (void *_this, void *self, const char *descriptor, size_t hash, void *class_loader, void *dex_file, void *dex_class_def) = 0;

//[<=5.1]bool ClassLinker::InitializeClass(              Handle<mirror::Class> klass, bool can_init_statics, bool can_init_parents)
static char  new_ART_ClassLinker_InitializeClass_5 (void *thiz,             void *klass, char can_init_statics, char can_init_parents);
static char(*old_ART_ClassLinker_InitializeClass_5)(void *thiz,             void *klass, char can_init_statics, char can_init_parents) = 0;

//[>=6.0]bool ClassLinker::InitializeClass(Thread* self, Handle<mirror::Class> klass, bool can_init_statics, bool can_init_parents)
static char  new_ART_ClassLinker_InitializeClass   (void *thiz, void *self, void *klass, char can_init_statics, char can_init_parents);
static char(*old_ART_ClassLinker_InitializeClass)  (void *thiz, void *self, void *klass, char can_init_statics, char can_init_parents) = 0;

static void    JNI_printlog(JNIEnv *env, jclass clazz, int bufid, int prio, jstring tag, jstring msg)
{
    CDebugUtil::AndroidLogBufWrite(bufid, prio, CJavaToBase(env, tag), "%s", CJavaToBase(env, msg).GetValue());
}

static jobject JNI_hookmethod(JNIEnv *env, jclass clazz, jclass tweak_class, jstring tweak_method, jclass hook_class, jstring hook_method)
{
    jobject backup = CJavaHook::HookMethod(env, tweak_class, CJavaToBase(env, tweak_method), hook_class, CJavaToBase(env, hook_method));
    return backup;
}

extern "C" JNIEXPORT int JNI_OnLoad(JavaVM *vm, void *reserved)
{
    string package;
    CAppContext::GetPackageName(CJniEnv(vm), package);

    g_dexfile = JAVAHOOK_DEXDIR + package + "/" JAVAHOOK_DEXFILE;
    if(!CFileUtil::IsExist(g_dexfile.c_str()))
        return JNI_VERSION_1_4;

    //在非root环境下CJniUtil::MirrorToObject中的JNIEnvExt_NewLocalRef会获取失败
    void *h = CLinkerUtil::dlopen(string(CJniUtil::GetAndroidVmDir()+string("libart.so")).c_str(), 0);
    *(void **)&JNIEnvExt_NewLocalRef = dlsym(h, "_ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE");

    g_vm = vm;
    g_sdkver = CJniUtil::GetAndroidSdkVer();

    if(g_sdkver>=29 && !CJniUtil::Is64bit()) //>=10.0
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker11DefineClassEPNS_6ThreadEPKcjNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS_3dex8ClassDefE"), (size_t)new_ART_ClassLinker_DefineClass, (size_t *)&old_ART_ClassLinker_DefineClass);
    else if(g_sdkver>=29 && CJniUtil::Is64bit()) //>=10.0
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker11DefineClassEPNS_6ThreadEPKcmNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS_3dex8ClassDefE"), (size_t)new_ART_ClassLinker_DefineClass, (size_t *)&old_ART_ClassLinker_DefineClass);
    else if(g_sdkver>=22 && g_sdkver<=28 && !CJniUtil::Is64bit()) //5.1\6.0\7.0\7.1\8.0\8.1\9.0
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker11DefineClassEPNS_6ThreadEPKcjNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS9_8ClassDefE"), (size_t)new_ART_ClassLinker_DefineClass, (size_t *)&old_ART_ClassLinker_DefineClass);
    else if(g_sdkver>=22 && g_sdkver<=28 && CJniUtil::Is64bit()) //5.1\6.0\7.0\7.1\8.0\8.1\9.0
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker11DefineClassEPNS_6ThreadEPKcmNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS9_8ClassDefE"), (size_t)new_ART_ClassLinker_DefineClass, (size_t *)&old_ART_ClassLinker_DefineClass);
    else if(g_sdkver==21) //==5.0
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker11DefineClassEPKcNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS7_8ClassDefE"), (size_t)new_ART_ClassLinker_DefineClass_5_0, (size_t *)&old_ART_ClassLinker_DefineClass_5_0);

    if(g_sdkver <= 22) //5.0\5.1
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker15InitializeClassENS_6HandleINS_6mirror5ClassEEEbb"), (size_t)old_ART_ClassLinker_InitializeClass_5, (size_t *)&old_ART_ClassLinker_InitializeClass_5);
    else if(g_sdkver >= 23) //>=6.0
        CApiHook::HookFunction((size_t)dlsym(h, "_ZN3art11ClassLinker15InitializeClassEPNS_6ThreadENS_6HandleINS_6mirror5ClassEEEbb"), (size_t)new_ART_ClassLinker_InitializeClass, (size_t *)&old_ART_ClassLinker_InitializeClass);

    CJavaHook::InitJavaHook(vm);
    __javahook_initbridge__();
    return JNI_VERSION_1_4;
}

static void *  new_ART_ClassLinker_DefineClass_5_0 (void *_this,             const char *descriptor,              void *class_loader, void *dex_file, void *dex_class_def)
{
    void *hr = old_ART_ClassLinker_DefineClass_5_0(_this, descriptor, class_loader, dex_file, dex_class_def);
    __javahook_defineclass__(descriptor, class_loader, hr);
    return hr;
}

static void *  new_ART_ClassLinker_DefineClass     (void *_this, void *self, const char *descriptor, size_t hash, void *class_loader, void *dex_file, void *dex_class_def)
{
    void *hr = old_ART_ClassLinker_DefineClass(_this, self, descriptor, hash, class_loader, dex_file, dex_class_def);
    __javahook_defineclass__(descriptor, class_loader, hr);
    return hr;
}

static char  new_ART_ClassLinker_InitializeClass_5 (void *thiz,             void *klass, char can_init_statics, char can_init_parents)
{
    char hr = old_ART_ClassLinker_InitializeClass_5(thiz,       klass, can_init_statics, can_init_parents);
    __javahook_initializeclass__(hr?klass:0);
    return hr;
}

static char  new_ART_ClassLinker_InitializeClass   (void *thiz, void *self, void *klass, char can_init_statics, char can_init_parents)
{
    char hr = old_ART_ClassLinker_InitializeClass  (thiz, self, klass, can_init_statics, can_init_parents);
    __javahook_initializeclass__(hr?klass:0);
    return hr;
}

static int __javahook_initbridge__()
{
    if(g_JavaTweakCallback || g_initializing)
        return -1;
    CThreadLockHelper help(&g_lock);//这里要加锁，因为可能是多线程调用
    if(g_JavaTweakCallback || g_initializing)
        return -1;

    g_initializing = 1;
    CJniEnv env(g_vm);

    CJniObj callback(env, CJniUtil::ClassForName(env, JAVAHOOK_PACKAGE JAVAHOOK_CALLBACK));
    env->ExceptionClear();//这里一定要清理异常，因为callback可能是空（空是正常逻辑，因为还没有加载dex）
    if( !callback.GetObj() &&
        !CJavaHook::LoadDexFile(env, g_dexfile.c_str()))
    {
        g_initializing = 0;
        return -1;
    }
    callback.Attach(env, CJniUtil::ClassForName(env, JAVAHOOK_PACKAGE JAVAHOOK_CALLBACK));
    g_JavaTweakCallback = (jclass)env->NewGlobalRef(callback);
    g_defineJavaClass = env->GetStaticMethodID(g_JavaTweakCallback, "defineJavaClass", "(Ljava/lang/Class;)V");

    CJniObj bridge(env, CJniUtil::ClassForName(env, JAVAHOOK_PACKAGE JAVAHOOK_BRIDGE));
    JNINativeMethod nm[] =
    {
        {"nativePrintLog", "(IILjava/lang/String;Ljava/lang/String;)V", (void *)JNI_printlog},
        {"nativeHookMethod", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/reflect/Method;", (void *)JNI_hookmethod},
    };
    env->RegisterNatives(bridge, nm, sizeof(nm)/sizeof(JNINativeMethod));
    CDebugUtil::WriteToLogcat("javahook: loadDexFile = %s, JavaTweakCallback = %p, defineJavaClass = %p\r\n", g_dexfile.c_str(), g_JavaTweakCallback, g_defineJavaClass);
    g_initializing = 0;
    return 0;
}

static int __javahook_defineclass__(const char *descriptor, void *class_loader, void *new_class)
{
    if(!new_class)
        return -1;
    if(!g_defineJavaClass || strstr(descriptor, JAVAHOOK_PREFIX))
        return -1; //防止死锁

    g_lock.Lock();
    g_initclass[(size_t)new_class] = 1;
    g_lock.UnLock();

    CJniEnv env(g_vm);
    CJniObj clazz(env, JNIEnvExt_NewLocalRef(env, new_class));

    env->CallStaticVoidMethod(g_JavaTweakCallback, g_defineJavaClass, clazz.GetObj());
    return 0;
}

static int __javahook_initializeclass__(void *klass)
{
    if(klass) //Handle<mirror::Class>
        klass = *(void **)klass;
    if(klass && g_sdkver<=25) //8.0及以上版本无需二次解引
        klass = *(void **)klass;
    if(CJniUtil::Is64bit()) //解引后的对象是一个uint32_t数据，在64位模式下需要清除高位
        klass = (void *)((size_t)klass & 0xffffffff);
    if(!klass || !g_defineJavaClass)
        return -1;

    int is_first_init = 0;
    g_lock.Lock();
    if(g_initclass.find((size_t)klass) == g_initclass.end())
    {
        is_first_init = 1;
        g_initclass[(size_t)klass] = is_first_init;
    }
    g_lock.UnLock();
    if(!is_first_init)
    {
        return -1; //确保只回调一次，因为类可能多次被初始化
    }
    CJniEnv env(g_vm);
    CJniObj clazz(env, JNIEnvExt_NewLocalRef(env, klass));

    CJniObj jname(env, CJniCall::CallObjectMethod(env, clazz, "getName", "()Ljava/lang/String;").l);
    if(strstr(CJavaToBase(env, (jstring)jname.GetObj()), JAVAHOOK_PACKAGE))
    {
        return -1; //防止死锁
    }
    env->CallStaticVoidMethod(g_JavaTweakCallback, g_defineJavaClass, clazz.GetObj());
    return 0;
}
