//JavaHook.cpp

#include "JavaHook.h"
#include "HookTrampoline.h"
#include "hook_trampoline_arm.h"
#include "../include/JniUtil.h"
#include "../include/LinkerUtil.h"
//#include "../include/DebugUtil.h"

static const char*  g_sdkverStr[] = {"KitKat_4_4", "KitKat_4_4W", "Lollipop_5_0", "Lollipop_5_1", "Marshmallow_6_0", "Nougat_7_0", "Nougat_7_1", "Oreo_8_0", "Oreo_8_1", "Pie_9_0", "AndroidQ_10_0", "AndroidR_11_0"};
static void*        (*ART_Runtime_CreateResolutionMethod)(void* thiz) = 0; //ArtMethod* Runtime::CreateResolutionMethod();
static void*        g_runtime = 0;
static void*        g_ArtQuickToInterpreterBridge = 0;
static void*        g_ArtQuickGenericJniTrampoline = 0;
static void*        g_ArtJniDlsymLookupStub = 0;
static unsigned     g_sizeofArtMethod = 0;
static unsigned     g_offsetAccessFlags = 0;
static unsigned     g_offsetHotnessCount = 0;
static unsigned     g_offsetInterpreterCode = 0;
static unsigned     g_offsetJniCode = 0;
static unsigned     g_offsetCompiledCode = 0;
static unsigned     g_kAccCompileDontBother = 0;
static unsigned     g_sdkver = 0;

CJavaHook::CJavaHook()
{
}

CJavaHook::~CJavaHook()
{
}

BOOL CJavaHook::InitJavaHook(JavaVM *vm)
{
    if(!g_sdkver)
    {
        g_sdkver = CJniUtil::GetAndroidSdkVer();
    }
    if(g_sdkver < Lollipop_5_0)
    {
        return FALSE;
    }
    if(!vm)
    {
        vm = CJniUtil::GetJavaVM();
    }
    if(!g_sizeofArtMethod)
    {
        CJniEnv env(vm);
        CJniObj obj1(env, FindClassMethod(env, "java.lang.Object", "notify()"));
        CJniObj obj2(env, FindClassMethod(env, "java.lang.Object", "notifyAll()"));

        jmethodID mid1 = env->FromReflectedMethod(obj1);
        jmethodID mid2 = env->FromReflectedMethod(obj2);

        g_sizeofArtMethod = (unsigned)mid2 - (unsigned)mid1;
    }
    if(!g_runtime)
    {
        g_runtime = *(void **)(vm + 1);
    }
    if(!ART_Runtime_CreateResolutionMethod)
    {
        //ART_Runtime_CreateResolutionMethod函数所有版本虚拟机都会导出，且接口参数一致
        *(void **)&ART_Runtime_CreateResolutionMethod = CLinkerUtil::dlsym("libart.so", "_ZN3art7Runtime22CreateResolutionMethodEv");
    }
    if(!g_ArtQuickToInterpreterBridge)
    {
        *(void **)&g_ArtQuickToInterpreterBridge = CLinkerUtil::elfsym("libart.so", "art_quick_to_interpreter_bridge");
    }
    if(!g_ArtQuickGenericJniTrampoline)
    {
        *(void **)&g_ArtQuickGenericJniTrampoline = CLinkerUtil::elfsym("libart.so", "art_quick_generic_jni_trampoline");
    }
    if(!g_ArtJniDlsymLookupStub)
    {
        *(void **)&g_ArtJniDlsymLookupStub = CLinkerUtil::elfsym("libart.so", "art_jni_dlsym_lookup_stub");
    }
    if(!g_offsetCompiledCode)
    {
        if(g_sdkver >= Lollipop_5_1)
        {
            //5.0之后entry_point_from_quick_compiled_code_字段是最后一个字段
            g_offsetCompiledCode = g_sizeofArtMethod - sizeof(void*);
        }
        else//5.0
        {
            g_offsetCompiledCode = 40;
        }
    }
    if(!g_offsetJniCode)
    {
        //entry_point_from_quick_compiled_code_字段的前一个字段就是entry_point_from_jni_字段
        if(g_sdkver >= Lollipop_5_1)
        {
            g_offsetJniCode = g_offsetCompiledCode - sizeof(void*);//5.0之后系统中entry_point_from_quick_compiled_code_字段类型是void*
        }
        else//5.0
        {
            g_offsetJniCode = g_offsetCompiledCode - sizeof(uint64_t);//5.0系统中entry_point_from_quick_compiled_code_字段类型是uint64_t
        }
    }
    if(g_sdkver <= Marshmallow_6_0)
    {
        //6.0及以前的版本才有entry_point_from_interpreter_字段
        if(!g_offsetInterpreterCode)
        {
            if(g_sdkver >= Lollipop_5_1)
            {
                g_offsetInterpreterCode = g_offsetJniCode - sizeof(void*);//5.0之后系统中entry_point_from_jni_字段类型是void*
            }
            else//5.0
            {
                g_offsetInterpreterCode = g_offsetJniCode - sizeof(uint64_t);//5.0系统中entry_point_from_jni_字段类型是uint64_t
            }
        }
    }
    if(!g_offsetAccessFlags)
    {
        CJniEnv env(vm);
        CJniObj toStringMethod(env, FindClassMethod(env, "java.lang.Object", "toString()"));

        uint32_t* artMethod = (uint32_t *)env->FromReflectedMethod(toStringMethod);
        if(g_sdkver <= Lollipop_5_1)
        {
            //5.0 5.1 系统中Method中只有artMethod一个字段，并且artMethod对象在JAVA层是透明的
            toStringMethod.Attach(env, CJniUtil::GetObjectField(env, toStringMethod, "artMethod", "Ljava/lang/reflect/ArtMethod;").l);
        }
        uint32_t accessFlags = CJniUtil::GetObjectField(env, toStringMethod, "accessFlags", "I").i;

        for(int i = 0; i < g_sizeofArtMethod/sizeof(uint32_t); ++i)
        {
            if(artMethod[i] == accessFlags)
            {
                g_offsetAccessFlags = i * sizeof(uint32_t);
                break;
            }
        }
    }
    if(g_sdkver >= Nougat_7_0)
    {
        //7.0及以后的版本才有hotness_count_字段
        if(!g_offsetHotnessCount)
        {
            CJniEnv env(vm);
            CJniObj toStringMethod(env, FindClassMethod(env, "java.lang.Object", "toString()"));

            uint32_t* artMethod = (uint32_t *)env->FromReflectedMethod(toStringMethod);
            uint32_t dexMethodIndex = CJniUtil::GetObjectField(env, toStringMethod, "dexMethodIndex", "I").i;

            for(int i = 0; i < g_sizeofArtMethod/sizeof(uint32_t); ++i)
            {
                if(artMethod[i] == dexMethodIndex)
                {
                    g_offsetHotnessCount = (i+1) * sizeof(uint32_t) + sizeof(uint16_t);
                    break;
                }
            }
        }
    }
    if(g_sdkver>=Nougat_7_0 && g_sdkver<=Oreo_8_0)
    {
        g_kAccCompileDontBother = 0x01000000;
    }
    else if(g_sdkver>=Oreo_8_1)
    {
        g_kAccCompileDontBother = 0x02000000;
    }
    //CDebugUtil::WriteToLogcat("sdkverStr=%s, runtime=%p, ArtQuickGenericJniTrampoline=%p, ArtQuickToInterpreterBridge=%p, ArtJniDlsymLookupStub=%p, CreateResolutionMethod=%p, sizeofArtMethod=%d, offsetAccessFlags=%d, offsetHotnessCount=%d, offsetInterpreterCode=%d, offsetJniCode=%d, offsetCompiledCode=%d\r\n", g_sdkverStr[g_sdkver-__ANDROID_API_MIN__], g_runtime, g_ArtQuickGenericJniTrampoline, g_ArtQuickToInterpreterBridge, g_ArtJniDlsymLookupStub, ART_Runtime_CreateResolutionMethod, g_sizeofArtMethod, g_offsetAccessFlags, g_offsetHotnessCount, g_offsetInterpreterCode, g_offsetJniCode, g_offsetCompiledCode);
    return TRUE;
}

jobject CJavaHook::FindClassMethod(JNIEnv *env, jclass clazz, const char *method)
{
    //method: doCommandNative(int,java.lang.Object[])
    //method: doCommandNative
    if(!env || !clazz)
        return 0;
    if(!method || !*method)
        return 0;

    CJniObj arrObj(env, CJniUtil::CallObjectMethod(env, clazz, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;").l);
    if(!arrObj.GetObj())
        return 0;

    size_t mlen = strlen(method);
    jsize len = env->GetArrayLength(arrObj);
    int isname = !strchr(method, '('); //支持只匹配方法名称，不匹配参数

    for(jsize i = 0; i < len; ++i)
    {
        CJniObj ele(env, env->GetObjectArrayElement(arrObj, i));
        CJniObj strObj(env, CJniUtil::CallObjectMethod(env, ele, isname?"getName":"toString", "()Ljava/lang/String;").l);

        const char *jv = env->GetStringUTFChars(strObj, 0);
        string tostring = jv; //public static transient native java.lang.Object com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int,java.lang.Object[]) throws org.json.JSONException
        env->ReleaseStringUTFChars(strObj, jv);

        if(!isname)
        {
            string declared = tostring.substr(0, tostring.rfind(')')+1); //public static transient native java.lang.Object com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int,java.lang.Object[])
            if(declared.size()<=mlen || declared[declared.size()-mlen-1]!='.')
                continue;
            string method_ = declared.substr(declared.size() - mlen);
            if(method_ != method)
                continue;
        }
        else
        {
            if(tostring != method)
                continue;
        }
        return ele.Detach();
    }
    return 0;
}

jobject CJavaHook::FindClassMethod(JNIEnv *env, const char *clazz, const char *method)
{
    //java: Object com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int cmd, Object... args)
    //sign: Lcom/taobao/wireless/security/adapter/JNICLibrary;->(I[Ljava/lang/Object;)Ljava/lang/Object;
    //clazz: com.taobao.wireless.security.adapter.JNICLibrary
    //method: doCommandNative(int,java.lang.Object[])
    if(!env)
        return 0;
    if(!clazz || !*clazz)
        return 0;
    if(!method || !*method)
        return 0;

    CJniObj clsObj(env, CJniUtil::ClassForName(env, clazz));
    if(!clsObj.GetObj())
        return 0;

    jobject obj = FindClassMethod(env, clsObj, method);
    return obj;
}

jobject CJavaHook::BackupMethod(JNIEnv *env, jobject method)
{
    if(!env || !method)
        return 0;

    if(FALSE)
    {
        //最简洁、全版本兼容的创建被份方法的代码，绕过JAVA层创建Method的各种限制，也不再依赖JavaTweakStub.java(8.0版本这种模式会有问题)
        jmethodID orig = env->FromReflectedMethod(method);
        jmethodID back = (jmethodID)ART_Runtime_CreateResolutionMethod(g_runtime);//创建ArtMethod
        memcpy(back, orig, g_sizeofArtMethod);

        CJniObj declaringClass(env, CJniUtil::CallObjectMethod(env, method, "getDeclaringClass", "()Ljava/lang/Class;").l);
        jobject backupMethod = env->ToReflectedMethod(declaringClass, back, 0);//创建Method
        return backupMethod;
    }
    CJniObj backupMethod;
    if(g_sdkver <= Lollipop_5_1)//5.0\5.1
    {
        //Class<?> artMethodClass = Class.forName("java.lang.reflect.ArtMethod");
        //Constructor<?> constructor = artMethodClass.getDeclaredConstructor();
        //constructor.setAccessible(true);
        //Object destArtMethod = constructor.newInstance();
        //Method newMethod = Method.class.getConstructor(artMethodClass).newInstance(destArtMethod);

        CJniObj artMethodClass(env, env->FindClass("java/lang/reflect/ArtMethod"));
        CJniObj methodClass(env, env->FindClass("java/lang/reflect/Method"));

        CJniObj destArtMethodConstructor(env, CJniUtil::CallObjectMethod(env, artMethodClass, "getDeclaredConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", 0).l);

        CJniUtil::CallObjectMethod(env, destArtMethodConstructor, "setAccessible", "(Z)V", 1);
        CJniObj destArtMethod(env, CJniUtil::CallObjectMethod(env, destArtMethodConstructor, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;", 0).l);

        CJniObj artMethodClass_(env, CJniUtil::NewObjectArray(env, "java/lang/Class", 1, artMethodClass.GetObj()));
        CJniObj backupMethodConstructor(env, CJniUtil::CallObjectMethod(env, methodClass, "getConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", artMethodClass_.GetObj()).l);

        CJniObj destArtMethod_(env, CJniUtil::NewObjectArray(env, "java/lang/Object", 1, destArtMethod.GetObj()));
        backupMethod.Attach(env, CJniUtil::CallObjectMethod(env, backupMethodConstructor, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;", destArtMethod_.GetObj()).l);
    }
    else if(g_sdkver == Marshmallow_6_0)//6.0
    {
        //Constructor<Method> constructor = Method.class.getDeclaredConstructor();
        //AccessibleObject.setAccessible(new AccessibleObject[]{constructor}, true);
        //Method newMethod = constructor.newInstance();

        CJniObj methodClass(env, env->FindClass("java/lang/reflect/Method"));
        CJniObj destMethodConstructor(env, CJniUtil::CallObjectMethod(env, methodClass, "getDeclaredConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", 0).l);

        CJniObj accObj(env, CJniUtil::NewObjectArray(env, "java/lang/reflect/AccessibleObject", 1, destMethodConstructor.GetObj()));
        CJniUtil::CallClassMethod(env, "java/lang/reflect/AccessibleObject", "setAccessible", "([Ljava/lang/reflect/AccessibleObject;Z)V", accObj.GetObj(), 1);

        backupMethod.Attach(env, CJniUtil::CallObjectMethod(env, destMethodConstructor, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;", 0).l);

        jvalue artMethod;
        artMethod.j = (jlong)ART_Runtime_CreateResolutionMethod(g_runtime);//新创建的方法artMethod字段是空的，这里要创建ArtMethod，给artMethod字段赋值
        CJniUtil::SetObjectField(env, backupMethod.GetObj(), "artMethod", "J", artMethod);
    }
    else
    {
        //Constructor<Method> constructor = Method.class.getDeclaredConstructor();
        //AccessibleObject.setAccessible(new AccessibleObject[]{constructor}, true);这句调用在7.0上会抛出异常，无法构造方法对象
        backupMethod.Attach(env, CJniUtil::CallClassMethod(env, "com/android/guobao/liao/apptweak/JavaTweakStub", "getStubMethod", "()Ljava/lang/reflect/Method;").l);
    }
    CJniObj abstractMethodClass(env, env->FindClass(g_sdkver<=Nougat_7_1?"java/lang/reflect/AbstractMethod":"java/lang/reflect/Executable"));
    CJniObj methodFields(env, CJniUtil::CallObjectMethod(env, abstractMethodClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;").l);

    jsize len = env->GetArrayLength(methodFields);
    for(jsize i = 0; i < len; ++i)
    {
        CJniObj methodField(env, env->GetObjectArrayElement(methodFields, i));

        CJniObj fieldName(env, CJniUtil::CallObjectMethod(env, methodField, "getName", "()Ljava/lang/String;").l);
        CJavaToBase _fieldName(env, (jstring)fieldName.GetObj());
        if(!strcmp((char *)_fieldName.GetValue(), "artMethod"))
        {
            //artMethod字段的值就是ArtMethod*,后续会调用memcpy(back, orig, g_sizeofArtMethod)，所以这里指针不需要替换
            continue;
        }
        CJniUtil::CallObjectMethod(env, methodField, "setAccessible", "(Z)V", 1);
        CJniObj fieldValue(env, CJniUtil::CallObjectMethod(env, methodField, "get", "(Ljava/lang/Object;)Ljava/lang/Object;", method).l);
        CJniUtil::CallObjectMethod(env, methodField, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V", backupMethod.GetObj(), fieldValue.GetObj());
    }

    //下面这段是将原方法的ArtMethod结构体字段完全拷贝到备份方法中，使得备份方法和原方法完全一致
    jmethodID orig = env->FromReflectedMethod(method);
    jmethodID back = env->FromReflectedMethod(backupMethod);
    memcpy(back, orig, g_sizeofArtMethod);

    return backupMethod.Detach();
}

BOOL CJavaHook::ModifyMethod(JNIEnv *env, jobject method_orig, jobject method_back, jobject method_hook, int is_uninit)
{
    if(!env || !method_orig || !method_back || !method_hook)
        return FALSE;

    //设置备份方法可被正常调用，因为原始方法可能是私有或保护方法，外部不能直接调用
    CJniUtil::CallObjectMethod(env, method_back, "setAccessible", "(Z)V", 1);

    jmethodID orig = env->FromReflectedMethod(method_orig);
    jmethodID back = env->FromReflectedMethod(method_back);
    jmethodID hook = env->FromReflectedMethod(method_hook);
    if(!orig || !back || !hook)
        return FALSE;

    if(g_sdkver >= Nougat_7_0)
    {
        //kAccCompileDontBother(0x01000000),禁用 JIT
        *(unsigned *)((char *)back + g_offsetAccessFlags) |= g_kAccCompileDontBother;
        *(unsigned *)((char *)orig + g_offsetAccessFlags) |= g_kAccCompileDontBother;
    }
    int isAbstract = *(unsigned *)((char *)orig + g_offsetAccessFlags) & kAccAbstract;
    if((g_sdkver>=Nougat_7_0 && g_sdkver<=Pie_9_0) || (g_sdkver>=AndroidQ_10_0 && !isAbstract))
    {
        //hotness_count_=0
        *(unsigned short *)((char *)back + g_offsetHotnessCount) = 0;
        *(unsigned short *)((char *)orig + g_offsetHotnessCount) = 0;
    }
    if(g_sdkver >= Oreo_8_0)
    {
        // set the target method to native so that Android O wouldn't invoke it with interpreter
        *(unsigned *)((char *)back + g_offsetAccessFlags) |= kAccNative;
        *(unsigned *)((char *)orig + g_offsetAccessFlags) |= kAccNative;
    }
    if(g_sdkver >= Oreo_8_1)
    {
        *(unsigned *)((char *)back + g_offsetAccessFlags) |= kAccPreviouslyWarm;
        *(unsigned *)((char *)orig + g_offsetAccessFlags) |= kAccPreviouslyWarm;
    }
    if(g_sdkver >= AndroidQ_10_0)
    {
        // On Android Q whether to use the fast path or not is cached in the ART method structure
        *(unsigned *)((char *)back + g_offsetAccessFlags) &= ~kAccFastInterpreterToInterpreterInvoke;
        *(unsigned *)((char *)orig + g_offsetAccessFlags) &= ~kAccFastInterpreterToInterpreterInvoke;
    }
    if(is_uninit)
    {
        //对于没有初始化完成的方法（方法未编译，JNI未注册好）必须设置跳板
        const unsigned char *tramp_backup = CHookTrampoline::GetInstance()->CreateBackupTrampoline(orig, *(void **)((char *)orig + g_offsetCompiledCode));
        *(unsigned *)((char *)back + g_offsetCompiledCode) = (unsigned)tramp_backup;
    }
    if(g_sdkver <= Marshmallow_6_0)
    {
        //7.0及以后的版本去掉了entry_point_from_interpreter_字段
        *(unsigned *)((char *)back + g_offsetInterpreterCode) = *(unsigned *)((char *)orig + g_offsetInterpreterCode);
        *(unsigned *)((char *)orig + g_offsetInterpreterCode) = *(unsigned *)((char *)hook + g_offsetInterpreterCode);
    }
    const unsigned char *tramp_origin = CHookTrampoline::GetInstance()->CreateOriginTrampoline(hook, (unsigned char)g_offsetCompiledCode);
    *(unsigned *)((char *)orig + g_offsetCompiledCode) = (unsigned)tramp_origin;

    //确保备份方法为私有方法，修复如下异常：java.lang.IllegalArgumentException: Wrong number of arguments; expected 1, got 0
    *(unsigned *)((char *)back + g_offsetAccessFlags) &= ~(kAccPublic | kAccProtected);//~(ACC_PUBLIC | ACC_PROTECTED)
    *(unsigned *)((char *)back + g_offsetAccessFlags) |= kAccPrivate;//ACC_PRIVATE

    //设置自定义标记，防止重复hook
    *(unsigned *)((char *)back + g_offsetAccessFlags) |= kAccTweakHookedMethod;
    *(unsigned *)((char *)orig + g_offsetAccessFlags) |= kAccTweakHookedMethod;
    return TRUE;
}

jobject CJavaHook::HookMethod(JNIEnv *env, jclass tweak_class, const char *tweak_method, jclass hook_class, const char *hook_method)
{
    if(!env || !tweak_class || !hook_class)
        return 0;
    if(!tweak_method || !*tweak_method)
        return 0;
    if(!hook_method || !*hook_method)
        return 0;

    CJniObj tweak(env, FindClassMethod(env, tweak_class, tweak_method));
    if(!tweak.GetObj())
        return 0;

    jmethodID old_ = env->FromReflectedMethod(tweak);
    if(*(unsigned *)((char *)old_ + g_offsetAccessFlags) & kAccTweakHookedMethod) //备份方法或者跳板方法不能hook
        return 0;

    CJniObj hook(env, FindClassMethod(env, hook_class, hook_method));
    if(!hook.GetObj())
        return 0;

    jmethodID new_ = env->FromReflectedMethod(hook);
    if(*(unsigned *)((char *)new_ + g_offsetAccessFlags) & kAccTweakHookedMethod) //备份方法或者跳板方法不能hook
        return 0;

    CJniObj backup(env, BackupMethod(env, tweak));
    if(!backup.GetObj())
        return 0;

    int is_uninit = 0;
    void *entry_point_from_quick_compiled_code_ = *(void **)((char *)old_ + g_offsetCompiledCode);
    if( entry_point_from_quick_compiled_code_==g_ArtQuickToInterpreterBridge ||
        entry_point_from_quick_compiled_code_==g_ArtQuickGenericJniTrampoline)
        is_uninit = 1; //方法未编译

    unsigned is_native = *(unsigned *)((char *)old_ + g_offsetAccessFlags) & kAccNative;
    void *entry_point_from_jni_ = *(void **)((char *)old_ + g_offsetJniCode);
    if(is_native && entry_point_from_jni_==g_ArtJniDlsymLookupStub)
        is_uninit = 1; //JNI 未注册

    if(!ModifyMethod(env, tweak, backup, hook, is_uninit))
        return 0;

    return backup.Detach();
}

jobject CJavaHook::HookMethod(JNIEnv *env, const char *tweak_class, const char *tweak_method, const char *hook_class, const char *hook_method)
{
    if(!env)
        return 0;
    if(!tweak_class || !*tweak_class)
        return 0;
    if(!tweak_method || !*tweak_method)
        return 0;
    if(!hook_class || !*hook_class)
        return 0;
    if(!hook_method || !*hook_method)
        return 0;

    CJniObj tweak_class_(env, CJniUtil::ClassForName(env,  tweak_class));
    if(!tweak_class_.GetObj())
        return 0;

    CJniObj hook_class_(env, CJniUtil::ClassForName(env,  hook_class));
    if(!hook_class_.GetObj())
        return 0;

    jobject backup = HookMethod(env, tweak_class_, tweak_method, hook_class_, hook_method);
    return backup;
}

BOOL CJavaHook::LoadDexFile(JNIEnv *env, const char *dexfile, int opt)
{
    if(!env)
        return FALSE;
    if(!dexfile || !*dexfile)
        return FALSE;
    if(access(dexfile, 0))
        return FALSE;

    CJniObj loader(env, env->FindClass("dalvik/system/DexClassLoader"));//这个类是一个未加载的新类，会调用art::ClassLinker::DefineClass
    if(!loader.GetObj())
        return FALSE;

    string apptweak;
    if(!CJniUtil::CreateAppDir(env, "tweak", apptweak))
        return FALSE;

    CJniObj app(env, CJniUtil::GetCurrentApplication(env));
    if(!app.GetObj())
        return FALSE;

    CJniObj pdcl(env, CJniUtil::CallObjectMethod(env, app, "getClassLoader", "()Ljava/lang/ClassLoader;").l);
    if(!pdcl.GetObj())
        return FALSE;

    string tostring;
    CJniUtil::ObjectToString(env, pdcl, tostring);
    if(strstr(tostring.c_str(), dexfile))
        return FALSE;//已经加载完成了

    string oatfile = apptweak + strrchr(dexfile, '/');
    if(opt && g_sdkver<=__ANDROID_API_P__ && !OptDexFile(env, dexfile, oatfile.c_str()))
        return FALSE; //Android10及以后版本不再允许从应用进程调用dex2oat

    CBaseToJava dexPath(env, dexfile);
    CBaseToJava optPath(env, apptweak.c_str());
    CJniObj dcl(env, CJniUtil::NewClassObject(env, "dalvik/system/DexClassLoader", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V", dexPath.GetObject(), optPath.GetObject(), 0, pdcl.GetObj()));
    if(!dcl.GetObj())
        return FALSE;

    CJniObj pathList_loaded(env, CJniUtil::GetObjectField(env, pdcl, "pathList", "Ldalvik/system/DexPathList;").l);
    if(!pathList_loaded.GetObj())
        return FALSE;

    CJniObj dexElements_loaded(env, CJniUtil::GetObjectField(env, pathList_loaded, "dexElements", "[Ldalvik/system/DexPathList$Element;").l);
    if(!dexElements_loaded.GetObj())
        return FALSE;

    CJniObj pathList_loading(env, CJniUtil::GetObjectField(env, dcl, "pathList", "Ldalvik/system/DexPathList;").l);
    if(!pathList_loading.GetObj())
        return FALSE;

    CJniObj dexElements_loading(env, CJniUtil::GetObjectField(env, pathList_loading, "dexElements", "[Ldalvik/system/DexPathList$Element;").l);
    if(!dexElements_loading.GetObj())
        return FALSE;

    CJniObj elementClass(env, env->GetObjectClass(dexElements_loaded));
    CJniObj componentType(env, CJniUtil::CallObjectMethod(env, elementClass, "getComponentType", "()Ljava/lang/Class;").l);
    if(!componentType.GetObj())
        return FALSE;

    jsize len_loaded = env->GetArrayLength(dexElements_loaded);
    jsize len_new = len_loaded + 1;
    CJniObj dexElements_new(env, CJniUtil::CallClassMethod(env, "java/lang/reflect/Array", "newInstance", "(Ljava/lang/Class;I)Ljava/lang/Object;", componentType.GetObj(), len_new).l);
    if(!dexElements_new.GetObj())
        return FALSE;

    CJniObj dexElement_loading(env, env->GetObjectArrayElement(dexElements_loading, 0));
    env->SetObjectArrayElement(dexElements_new, 0, dexElement_loading);

    for(int i = 1; i < len_new; i++)
    {
        CJniObj dexElement_loaded(env, env->GetObjectArrayElement(dexElements_loaded, i-1));
        env->SetObjectArrayElement(dexElements_new, i, dexElement_loaded);
    }

    jvalue value;
    value.l = dexElements_new.GetObj();
    CJniUtil::SetObjectField(env, pathList_loaded, "dexElements", "[Ldalvik/system/DexPathList$Element;", value);
    return TRUE;
}

BOOL CJavaHook::OptDexFile(JNIEnv *env, const char *dexfile, const char *oatfile)
{
    if(!env)
        return FALSE;
    if(!dexfile || !*dexfile)
        return FALSE;
    if(!oatfile || !*oatfile)
        return FALSE;
    if(access(dexfile, 0))
        return FALSE;

    string pack;
    CJniUtil::GetPackageName(env, pack);
    if(pack.empty())
        return FALSE;

    char proc[1024];
    int fd = open("/proc/self/cmdline", O_RDONLY);
    int num = read(fd, proc, sizeof(proc));
    close(fd);
    if(num<=0 || !proc[0])
        return FALSE;

    if(pack != proc)
        return TRUE;//如果是后台进程，这里不需要优化，因为主进程已经优化过了

    char features[100], variant[100], Xms[100], Xmx[100];
    __system_property_get("dalvik.vm.isa.arm.features", features);
    __system_property_get("dalvik.vm.isa.arm.variant", variant);
    __system_property_get("dalvik.vm.dex2oat-Xms", Xms);
    __system_property_get("dalvik.vm.dex2oat-Xmx", Xmx);

    ///system/bin/dex2oat --runtime-arg -classpath --runtime-arg  --instruction-set=arm --instruction-set-features=smp,-div,-atomic_ldrd_strd --runtime-arg -Xrelocate --boot-image=/system/framework/boot.art --runtime-arg -Xms64m --runtime-arg -Xmx512m --instruction-set-variant=cortex-a9 --instruction-set-features=default --dex-file=/data/app/com.MobileTicket-1/base.apk --oat-file=/data/dalvik-cache/arm/data@app@com.MobileTicket-1@base.apk@classes.dex
    string cmdline("/system/bin/dex2oat");
    cmdline += " --runtime-arg -classpath";
    cmdline += " --runtime-arg \"&\""; //Special classpath that skips shared library check.
    cmdline += " --instruction-set=arm";
    cmdline += " --instruction-set-features=" + string(features);
    //cmdline += " --instruction-set-variant=" + string(variant);
    cmdline += " --compiler-filter=speed";
    cmdline += " --runtime-arg -Xrelocate";
    cmdline += " --boot-image=/system/framework/boot.art";
    cmdline += " --dex-file=" + string(dexfile);
    cmdline += " --oat-file=" + string(oatfile);
    cmdline += " --runtime-arg -Xms" + string(Xms);
    cmdline += " --runtime-arg -Xmx" + string(Xmx);

    if(g_sdkver >= Marshmallow_6_0)
    {
        //6.0及以后版本才有这个配置项
        cmdline += " --instruction-set-variant=" + string(variant);
    }
    if(g_sdkver >= Oreo_8_1)
    {
        //8.1及以后版本才有这个配置项
        cmdline += " --class-loader-context=\"&\"";
    }
    char buf[1024];
    cmdline += " 2>&1";
    //先优化一次，防止app带 --compiler-filter=verify-none --compiler-filter=interpret-only命令,导致方法不生成汇编代码
    FILE *pf = popen(cmdline.c_str(), "r");
    size_t once = fread(buf, 1, sizeof(buf), pf);
    pclose(pf);
    return !once;
}
