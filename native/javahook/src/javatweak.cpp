//javahook.cpp

#include "include/JniUtil.h"
#include "include/ThreadLock.h"
#include "include/LinkerUtil.h"
#include "JavaHook/JavaHook.h"

#define JAVAHOOK_PREFIX     "com/android/guobao/liao/apptweak/"
#define JAVAHOOK_DEXFILE    "/data/local/tmp/javatweak.dex"
#define JAVAHOOK_PACKAGE    "com.android.guobao.liao.apptweak."
#define JAVAHOOK_BRIDGE     "JavaTweakBridge"
#define JAVAHOOK_CALLBACK   "JavaTweakCallback"

static JavaVM          *g_vm = 0;
static jmethodID        g_defineJavaClass = 0;
static jclass           g_JavaTweakCallback = 0;
static int              g_sdkver = 0;
static map<size_t, int> g_loader;
static jobject        (*JNIEnvExt_NewLocalRef)(void *thiz, void* mirror) = 0;
static CThreadLock      g_lock;

static jobject JNI_loadclass(JNIEnv *env, jclass clazz, jstring class_name);
static jobject JNI_hookmethod(JNIEnv *env, jclass clazz, jclass tweak_class, jstring tweak_method, jclass hook_class, jstring hook_method);

extern "C" void MSHookFunction(void *symbol, void *hook, void **old);
extern "C" JNIEXPORT int JNI_OnLoad(JavaVM *vm, void *reserved);

static int __javahook_initbridge__();
static int __javahook_defineclass__(const char *descriptor, void *class_loader, void *new_class);

//[==5.0]mirror::Class* ClassLinker::DefineClass(              const char* descriptor,              Handle<mirror::ClassLoader> class_loader, const DexFile& dex_file, const DexFile::ClassDef& dex_class_def)
static void *  new_ART_ClassLinker_DefineClass_5_0 (void *_this,             const char *descriptor,              void *class_loader, void *dex_file, void *dex_class_def);
static void *(*old_ART_ClassLinker_DefineClass_5_0)(void *_this,             const char *descriptor,              void *class_loader, void *dex_file, void *dex_class_def) = 0;

//[>=5.1]mirror::Class* ClassLinker::DefineClass(Thread* self, const char* descriptor, size_t hash, Handle<mirror::ClassLoader> class_loader, const DexFile& dex_file, const DexFile::ClassDef& dex_class_def)
static void *  new_ART_ClassLinker_DefineClass     (void *_this, void *self, const char *descriptor, size_t hash, void *class_loader, void *dex_file, void *dex_class_def);
static void *(*old_ART_ClassLinker_DefineClass)    (void *_this, void *self, const char *descriptor, size_t hash, void *class_loader, void *dex_file, void *dex_class_def) = 0;

static jobject JNI_loadclass(JNIEnv *env, jclass clazz, jstring class_name)
{
    jobject clazz_ = 0;
    CJavaToBase class_name_(env, class_name);
    for(map<size_t, int>::iterator itr = g_loader.begin(); !clazz_ && itr!=g_loader.end(); ++itr)
    {
        CJniObj class_loader(env, JNIEnvExt_NewLocalRef(env, (void *)itr->first));
        clazz_ = CJniUtil::ClassForName(env, (char *)class_name_.GetValue(), 1, class_loader);
    }
    return clazz_;
}

static jobject JNI_hookmethod(JNIEnv *env, jclass clazz, jclass tweak_class, jstring tweak_method, jclass hook_class, jstring hook_method)
{
    CJavaToBase tweak_method_(env, tweak_method);
    CJavaToBase hook_method_(env, hook_method);

    jobject backup = CJavaHook::HookMethod(env, tweak_class, (char *)tweak_method_.GetValue(), hook_class, (char *)hook_method_.GetValue());
    return backup;
}

extern "C" JNIEXPORT int JNI_OnLoad(JavaVM *vm, void *reserved)
{
    //在非root环境下CJniUtil::MirrorToObject中的JNIEnvExt_NewLocalRef会获取失败
    *(void **)&JNIEnvExt_NewLocalRef = CLinkerUtil::dlsym("libart.so", "_ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE");

    g_vm = vm;
    g_sdkver = CJniUtil::GetAndroidSdkVer();

    CJavaHook::InitJavaHook(vm);

    if(g_sdkver >= 22) //5.1
        MSHookFunction((void *)CLinkerUtil::dlsym("libart.so", "_ZN3art11ClassLinker11DefineClassEPNS_6ThreadEPKcjNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS9_8ClassDefE"), (void *)new_ART_ClassLinker_DefineClass, (void **)&old_ART_ClassLinker_DefineClass);
    if(g_sdkver == 21) //5.0
        MSHookFunction((void *)CLinkerUtil::dlsym("libart.so", "_ZN3art11ClassLinker11DefineClassEPKcNS_6HandleINS_6mirror11ClassLoaderEEERKNS_7DexFileERKNS7_8ClassDefE"), (void *)new_ART_ClassLinker_DefineClass_5_0, (void **)&old_ART_ClassLinker_DefineClass_5_0);

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

static int __javahook_initbridge__()
{
    if(g_JavaTweakCallback)
        return 0;
    CThreadLockHelper help(&g_lock);//这里要加锁，因为可能是多线程调用
    if(g_JavaTweakCallback)
        return 0;

    CJniEnv env(g_vm);

    CJniObj callback(env, CJniUtil::ClassForName(env, JAVAHOOK_PACKAGE JAVAHOOK_CALLBACK));
    env->ExceptionClear();//这里一定要清理异常，因为callback可能是空（空是正常逻辑，因为还没有加载dex）
    if( !callback.GetObj() &&
        !CJavaHook::LoadDexFile(env, JAVAHOOK_DEXFILE))
        return -1;

    callback.Attach(env, CJniUtil::ClassForName(env, JAVAHOOK_PACKAGE JAVAHOOK_CALLBACK));
    if( !callback.GetObj())//这里再判断一次是否为空（dex加载完成后一般不会是空）
        return -1;

    g_JavaTweakCallback = (jclass)env->NewGlobalRef(callback);
    g_defineJavaClass = env->GetStaticMethodID(g_JavaTweakCallback, "defineJavaClass", "(Ljava/lang/ClassLoader;Ljava/lang/Class;)V");

    CJniObj bridge(env, CJniUtil::ClassForName(env, JAVAHOOK_PACKAGE JAVAHOOK_BRIDGE));
    JNINativeMethod nm[] =
    {
        {"nativeLoadClass", "(Ljava/lang/String;)Ljava/lang/Class;", (void *)JNI_loadclass},
        {"nativeHookMethod", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/reflect/Method;", (void *)JNI_hookmethod},
    };
    env->RegisterNatives(bridge, nm, sizeof(nm)/sizeof(JNINativeMethod));
    __android_log_print(ANDROID_LOG_INFO, "WriteToLogcat", "javahook: loadDexFile = %s, JavaTweakCallback = %p, defineJavaClass = %p\r\n", JAVAHOOK_DEXFILE, g_JavaTweakCallback, g_defineJavaClass);
    return 0;
}

static int __javahook_defineclass__(const char *descriptor, void *class_loader, void *new_class)
{
    if(!new_class)
        return -1;
    if(!strstr(descriptor, JAVAHOOK_PREFIX))
        __javahook_initbridge__();
    if(class_loader) //Handle<mirror::ClassLoader>
        class_loader = *(void **)class_loader;
    if(class_loader && g_sdkver<=25)//8.0及以上版本无需二次解引
        class_loader = *(void **)class_loader;
    if(class_loader && g_loader.find((size_t)class_loader)==g_loader.end())
        g_loader[(size_t)class_loader] = 0;
    if(!g_defineJavaClass || strstr(descriptor, JAVAHOOK_PREFIX))
        return -1;//防止死锁

    CJniEnv env(g_vm);
    CJniObj loader(env, JNIEnvExt_NewLocalRef(env, class_loader));
    CJniObj clazz(env, JNIEnvExt_NewLocalRef(env, new_class));

    env->CallStaticVoidMethod(g_JavaTweakCallback, g_defineJavaClass, loader.GetObj(), clazz.GetObj());
    return 0;
}
