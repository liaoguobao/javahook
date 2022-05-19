//JavaHook.cpp

#include "JavaHook.h"
#include "HookHelper.h"
#include "HookTrampoline.h"
#include "hook_trampoline_arm.h"
#include "../include/JniUtil.h"
#include "../include/LinkerUtil.h"
//#include "../include/DebugUtil.h"

static const char*  g_sdkverStr[] = {"Lollipop_5_0", "Lollipop_5_1", "Marshmallow_6_0", "Nougat_7_0", "Nougat_7_1", "Oreo_8_0", "Oreo_8_1", "Pie_9_0", "AndroidQ_10_0", "AndroidR_11_0", "AndroidS_12_0"};
static void*        (*Runtime_CreateResolutionMethod)(void* thiz) = 0; //ArtMethod* Runtime::CreateResolutionMethod();
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
    CHookHelper::InitHookHelper();
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
    if(!g_runtime)
    {
        g_runtime = *(void **)(vm + 1);
    }
    string vmso = CJniUtil::GetAndroidVmDir() + string("libart.so"); //�������Ҫָ������·��,��Ϊ�п��ܻ��ҵ�/system/fake-libs64/libart.so
    if(!Runtime_CreateResolutionMethod)
    {
        *(void **)&Runtime_CreateResolutionMethod = CLinkerUtil::dlsym(vmso.c_str(), "_ZN3art7Runtime22CreateResolutionMethodEv");
    }
    if(!g_ArtQuickToInterpreterBridge)
    {
        g_ArtQuickToInterpreterBridge = CLinkerUtil::elfsym(vmso.c_str(), "art_quick_to_interpreter_bridge");
    }
    if(!g_ArtQuickGenericJniTrampoline)
    {
        g_ArtQuickGenericJniTrampoline = CLinkerUtil::elfsym(vmso.c_str(), "art_quick_generic_jni_trampoline");
    }
    if(!g_ArtJniDlsymLookupStub)
    {
        g_ArtJniDlsymLookupStub = CLinkerUtil::elfsym(vmso.c_str(), "art_jni_dlsym_lookup_stub");
    }
    if(!g_sizeofArtMethod)
    {
        CJniEnv env(vm);
        CJniObj obj1(env, FindClassMethod(env, "java.lang.Object", "notify()"));
        CJniObj obj2(env, FindClassMethod(env, "java.lang.Object", "notifyAll()"));

        jmethodID mid1 = env->FromReflectedMethod(obj1);
        jmethodID mid2 = env->FromReflectedMethod(obj2);

        g_sizeofArtMethod = (unsigned)((size_t)mid2 - (size_t)mid1);
    }
    if(!g_offsetCompiledCode)
    {
        if(g_sdkver >= Lollipop_5_1)
        {
            //5.0֮��entry_point_from_quick_compiled_code_�ֶ������һ���ֶ�
            g_offsetCompiledCode = g_sizeofArtMethod - sizeof(void*);
        }
        else//5.0
        {
            g_offsetCompiledCode = 40;
        }
    }
    if(!g_offsetJniCode)
    {
        //entry_point_from_quick_compiled_code_�ֶε�ǰһ���ֶξ���entry_point_from_jni_�ֶ�
        if(g_sdkver >= Lollipop_5_1)
        {
            g_offsetJniCode = g_offsetCompiledCode - sizeof(void*);//5.0֮��ϵͳ��entry_point_from_quick_compiled_code_�ֶ�������void*
        }
        else//5.0
        {
            g_offsetJniCode = g_offsetCompiledCode - sizeof(uint64_t);//5.0ϵͳ��entry_point_from_quick_compiled_code_�ֶ�������uint64_t
        }
    }
    if(g_sdkver <= Marshmallow_6_0)
    {
        //6.0����ǰ�İ汾����entry_point_from_interpreter_�ֶ�
        if(!g_offsetInterpreterCode)
        {
            if(g_sdkver >= Lollipop_5_1)
            {
                g_offsetInterpreterCode = g_offsetJniCode - sizeof(void*);//5.0֮��ϵͳ��entry_point_from_jni_�ֶ�������void*
            }
            else//5.0
            {
                g_offsetInterpreterCode = g_offsetJniCode - sizeof(uint64_t);//5.0ϵͳ��entry_point_from_jni_�ֶ�������uint64_t
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
            //5.0 5.1 ϵͳ��Method��ֻ��artMethodһ���ֶΣ�����artMethod������JAVA����͸����
            toStringMethod.Attach(env, CJniCall::GetObjectField(env, toStringMethod, "artMethod", "Ljava/lang/reflect/ArtMethod;").l);
        }
        uint32_t accessFlags = CJniCall::GetObjectField(env, toStringMethod, "accessFlags", "I").i;

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
        //7.0���Ժ�İ汾����hotness_count_�ֶ�
        if(!g_offsetHotnessCount)
        {
            CJniEnv env(vm);
            CJniObj toStringMethod(env, FindClassMethod(env, "java.lang.Object", "toString()"));

            uint32_t* artMethod = (uint32_t *)env->FromReflectedMethod(toStringMethod);
            uint32_t dexMethodIndex = CJniCall::GetObjectField(env, toStringMethod, "dexMethodIndex", "I").i;

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
    //CDebugUtil::WriteToLogcat("sdkverStr=%s, runtime_=%p, ArtQuickGenericJniTrampoline=%p, ArtQuickToInterpreterBridge=%p, ArtJniDlsymLookupStub=%p, CreateResolutionMethod=%p, sizeofArtMethod=%d, offsetAccessFlags=%d, offsetHotnessCount=%d, offsetInterpreterCode=%d, offsetJniCode=%d, offsetCompiledCode=%d\r\n", g_sdkverStr[g_sdkver-__ANDROID_API_MIN__], g_runtime, g_ArtQuickGenericJniTrampoline, g_ArtQuickToInterpreterBridge, g_ArtJniDlsymLookupStub, Runtime_CreateResolutionMethod, g_sizeofArtMethod, g_offsetAccessFlags, g_offsetHotnessCount, g_offsetInterpreterCode, g_offsetJniCode, g_offsetCompiledCode);
    return TRUE;
}

jobject CJavaHook::FindClassMethod(JNIEnv *env, jclass clazz, const char *method)
{
    //method: doCommandNative(int,java.lang.Object[]) //ͨ�������������ҷǹ��췽��
    //method: doCommandNative //ͨ�����Ʋ��ҷǹ��췽��
    //method: (java.lang.String) //ͨ�������������ҹ��췽��
    //method: (java.lang.String)java.util.List<java.lang.String> //ͨ������ǩ�����ҷǹ��췽��(Ӧ�����ֺ��������ܱ������ķ���)
    if(!env || !clazz)
        return 0;
    if(!method || !*method)
        return 0;

    size_t mlen = strlen(method);
    int isname = !strchr(method, '('); //ͨ�����Ʋ��ҷǹ��췽��
    int isinit = (method[0]=='(' && method[mlen-1]==')'); //���ҹ��췽��
    int issign = (method[0]=='(' && method[mlen-1]!=')'); //ͨ������ǩ�����ҷǹ��췽��

    CJniObj arrObj(env, !isinit?CJniCall::CallObjectMethod(env, clazz, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;").l:CJniCall::CallObjectMethod(env, clazz, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;").l);
    if(!arrObj.GetObj())
        return 0;

    string join = method;
    if(!isname && !issign)
    {
        CJniObj jname(env, CJniCall::CallObjectMethod(env, clazz, "getName", "()Ljava/lang/String;").l);

        join  = CJavaToBase(env, (jstring)jname.GetObj());
        join += (isinit ? "" : ".");
        join += method;
    }

    jsize len = env->GetArrayLength(arrObj);
    for(jsize i = 0; i < len; ++i)
    {
        CJniObj ele(env, env->GetObjectArrayElement(arrObj, i));
        CJniObj strObj(env, CJniCall::CallObjectMethod(env, ele, isname?"getName":(issign?"toGenericString":"toString"), "()Ljava/lang/String;").l);

        string decl(CJavaToBase(env, (jstring)strObj.GetObj())); //public static transient native java.lang.Object com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int,java.lang.Object[]) throws org.json.JSONException
        if(issign)
        {
            decl = decl.substr(0, decl.rfind(')')+1); //public static transient native java.lang.Object com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int,java.lang.Object[])
            string ret = decl.substr(0, decl.rfind(' ')); //public static transient native java.lang.Object
            ret = ret.substr(ret.rfind(' ')+1); //java.lang.Object
            decl = decl.substr(decl.rfind('(')); //(int,java.lang.Object[])
            decl += ret; //(int,java.lang.Object[])java.lang.Object
        }
        else if(!isname)
        {
            decl = decl.substr(0, decl.rfind(')')+1); //public static transient native java.lang.Object com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int,java.lang.Object[])
            decl = decl.substr(decl.rfind(' ')+1); //com.taobao.wireless.security.adapter.JNICLibrary.doCommandNative(int,java.lang.Object[])
        }
        if(decl == join)
        {
            return ele.Detach();
        }
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

        CJniObj destArtMethodConstructor(env, CJniCall::CallObjectMethod(env, artMethodClass, "getDeclaredConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", 0).l);

        CJniCall::CallObjectMethod(env, destArtMethodConstructor, "setAccessible", "(Z)V", 1);
        CJniObj destArtMethod(env, CJniCall::CallObjectMethod(env, destArtMethodConstructor, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;", 0).l);

        CJniObj artMethodClass_(env, CJniUtil::NewObjectArray(env, "java/lang/Class", 1, artMethodClass.GetObj()));
        CJniObj backupMethodConstructor(env, CJniCall::CallObjectMethod(env, methodClass, "getConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", artMethodClass_.GetObj()).l);

        CJniObj destArtMethod_(env, CJniUtil::NewObjectArray(env, "java/lang/Object", 1, destArtMethod.GetObj()));
        backupMethod.Attach(env, CJniCall::CallObjectMethod(env, backupMethodConstructor, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;", destArtMethod_.GetObj()).l);
    }
    else if(g_sdkver >= Marshmallow_6_0)//6.0
    {
        //Constructor<Method> constructor = Method.class.getDeclaredConstructor();
        //Field override = AccessibleObject.class.getDeclaredField(Build.VERSION.SDK_INT == Build.VERSION_CODES.M ? "flag" : "override");
        //override.setAccessible(true);
        //override.setBoolean(constructor, true);
        //Method newMethod = constructor.newInstance();

        CJniObj methodClass(env, env->FindClass("java/lang/reflect/Method"));
        CJniObj destMethodConstructor(env, CJniCall::CallObjectMethod(env, methodClass, "getDeclaredConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", 0).l);

        CJniObj accClass(env, env->FindClass("java/lang/reflect/AccessibleObject"));
        CJniObj overrideField(env, CJniCall::CallObjectMethod(env, accClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", CBaseToJava(env, g_sdkver==Marshmallow_6_0?"flag":"override").GetObject()).l);

        CJniCall::CallObjectMethod(env, overrideField, "setAccessible", "(Z)V", 1);
        CJniCall::CallObjectMethod(env, overrideField, "setBoolean", "(Ljava/lang/Object;Z)V", destMethodConstructor.GetObj(), 1);

        backupMethod.Attach(env, CJniCall::CallObjectMethod(env, destMethodConstructor, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;", 0).l);

        jvalue artMethod;
        artMethod.j = (jlong)Runtime_CreateResolutionMethod(g_runtime);//�´����ķ���artMethod�ֶ��ǿյģ�����Ҫ����ArtMethod����artMethod�ֶθ�ֵ
        CJniCall::SetObjectField(env, backupMethod.GetObj(), "artMethod", "J", artMethod);
    }
    CJniObj abstractMethodClass(env, env->FindClass(g_sdkver<=Nougat_7_1?"java/lang/reflect/AbstractMethod":"java/lang/reflect/Executable"));
    CJniObj methodFields(env, CJniCall::CallObjectMethod(env, abstractMethodClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;").l);

    jsize len = env->GetArrayLength(methodFields);
    for(jsize i = 0; i < len; ++i)
    {
        CJniObj methodField(env, env->GetObjectArrayElement(methodFields, i));
        CJniObj fieldName(env, CJniCall::CallObjectMethod(env, methodField, "getName", "()Ljava/lang/String;").l);

        if(!strcmp(CJavaToBase(env, (jstring)fieldName.GetObj()), "artMethod"))
        {
            //artMethod�ֶε�ֵ����ArtMethod*,���������memcpy(back, orig, g_sizeofArtMethod)����������ָ�벻��Ҫ�滻
            continue;
        }
        CJniCall::CallObjectMethod(env, methodField, "setAccessible", "(Z)V", 1);
        CJniObj fieldValue(env, CJniCall::CallObjectMethod(env, methodField, "get", "(Ljava/lang/Object;)Ljava/lang/Object;", method).l);
        CJniCall::CallObjectMethod(env, methodField, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V", backupMethod.GetObj(), fieldValue.GetObj());
    }

    //��������ǽ�ԭ������ArtMethod�ṹ���ֶ���ȫ���������ݷ����У�ʹ�ñ��ݷ�����ԭ������ȫһ��
    jmethodID orig = env->FromReflectedMethod(method);
    jmethodID back = env->FromReflectedMethod(backupMethod);
    memcpy(back, orig, g_sizeofArtMethod);

    return backupMethod.Detach();
}

BOOL CJavaHook::ModifyMethod(JNIEnv *env, jobject method_orig, jobject method_back, jobject method_hook, int is_uninit)
{
    if(!env || !method_orig || !method_back || !method_hook)
        return FALSE;

    //���ñ��ݷ����ɱ��������ã���Ϊԭʼ����������˽�л򱣻��������ⲿ����ֱ�ӵ���
    CJniCall::CallObjectMethod(env, method_back, "setAccessible", "(Z)V", 1);

    jmethodID orig = env->FromReflectedMethod(method_orig);
    jmethodID back = env->FromReflectedMethod(method_back);
    jmethodID hook = env->FromReflectedMethod(method_hook);
    if(!orig || !back || !hook)
        return FALSE;

    if(g_sdkver >= Nougat_7_0)
    {
        //kAccCompileDontBother(0x01000000),���� JIT
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
        //*(unsigned *)((char *)back + g_offsetAccessFlags) |= kAccNative; //���ݷ�������Ҫ���kAccNative��ǣ�����ᵼ��ĳЩ����hook�����
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
        //����û�г�ʼ����ɵķ���������δ���룬JNIδע��ã�������������
        const unsigned char *tramp_backup = CHookTrampoline::GetInstance()->CreateBackupTrampoline(orig, *(void **)((char *)orig + g_offsetCompiledCode));
        *(size_t *)((char *)back + g_offsetCompiledCode) = (size_t)tramp_backup;
    }
    if(g_sdkver <= Marshmallow_6_0)
    {
        //7.0���Ժ�İ汾ȥ����entry_point_from_interpreter_�ֶ�
        *(size_t *)((char *)back + g_offsetInterpreterCode) = *(size_t *)((char *)orig + g_offsetInterpreterCode);
        *(size_t *)((char *)orig + g_offsetInterpreterCode) = *(size_t *)((char *)hook + g_offsetInterpreterCode);
    }
    const unsigned char *tramp_origin = CHookTrampoline::GetInstance()->CreateOriginTrampoline(hook, (unsigned char)g_offsetCompiledCode);
    *(size_t *)((char *)orig + g_offsetCompiledCode) = (size_t)tramp_origin;

    //ȷ�����ݷ���Ϊ˽�з������޸������쳣��java.lang.IllegalArgumentException: Wrong number of arguments; expected 1, got 0
    *(unsigned *)((char *)back + g_offsetAccessFlags) &= ~(kAccPublic | kAccProtected);//~(ACC_PUBLIC | ACC_PROTECTED)
    *(unsigned *)((char *)back + g_offsetAccessFlags) |= kAccPrivate;//ACC_PRIVATE

    //�����Զ����ǣ���ֹ�ظ�hook
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
    if(*(unsigned *)((char *)old_ + g_offsetAccessFlags) & kAccTweakHookedMethod) //���ݷ����������巽������hook
        return 0;

    CJniObj hook(env, FindClassMethod(env, hook_class, hook_method));
    if(!hook.GetObj())
        return 0;

    jmethodID new_ = env->FromReflectedMethod(hook);
    if(*(unsigned *)((char *)new_ + g_offsetAccessFlags) & kAccTweakHookedMethod) //���ݷ����������巽������hook
        return 0;

    CJniObj backup(env, BackupMethod(env, tweak));
    if(!backup.GetObj())
        return 0;

    int is_uninit = 0;
    void *entry_point_from_quick_compiled_code_ = *(void **)((char *)old_ + g_offsetCompiledCode);
    if( entry_point_from_quick_compiled_code_==g_ArtQuickToInterpreterBridge ||
        entry_point_from_quick_compiled_code_==g_ArtQuickGenericJniTrampoline)
        is_uninit = 1; //����δ����

    unsigned is_native = *(unsigned *)((char *)old_ + g_offsetAccessFlags) & kAccNative;
    void *entry_point_from_jni_ = *(void **)((char *)old_ + g_offsetJniCode);
    if(is_native && entry_point_from_jni_==g_ArtJniDlsymLookupStub)
        is_uninit = 1; //JNI δע��

    //CDebugUtil::WriteToLogcat("init=%d, native=%d, ArtQuickToInterpreterBridge=%p, entry_point_from_quick_compiled_code_=%p, ArtQuickGenericJniTrampoline=%p, entry_point_from_jni_=%p, ArtJniDlsymLookupStub=%p\r\n", !is_uninit, !!is_native, g_ArtQuickToInterpreterBridge, entry_point_from_quick_compiled_code_, g_ArtQuickGenericJniTrampoline, entry_point_from_jni_, g_ArtJniDlsymLookupStub);
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

    CJniObj context(env, CAppContext::GetSystemContext(env));
    if(!context.GetObj())
        return FALSE;

    CJniObj pdcl(env, CJniCall::CallObjectMethod(env, context, "getClassLoader", "()Ljava/lang/ClassLoader;").l);
    if(!pdcl.GetObj())
        return FALSE;

    string tostring;
    CJniUtil::ObjectToString(env, pdcl, tostring);
    if(strstr(tostring.c_str(), dexfile))
        return FALSE;//�Ѿ����������

    string apptweak;
    if(!CAppContext::CreateAppDir(env, "tweak", apptweak))
        return FALSE;

    string oatfile = apptweak + string(strrchr(dexfile, '/')) + ".oat";
    if(opt && g_sdkver<=Pie_9_0 && !OptDexFile(env, dexfile, oatfile.c_str()))
        return FALSE; //Android10���Ժ�汾���������Ӧ�ý��̵���dex2oat

    CBaseToJava dexPath(env, dexfile);
    CBaseToJava optPath(env, apptweak.c_str());
    CJniObj dcl(env, CJniUtil::NewClassObject(env, "dalvik/system/DexClassLoader", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V", dexPath.GetObject(), optPath.GetObject(), 0, pdcl.GetObj()));
    if(!dcl.GetObj())
        return FALSE;

    CJniObj pathList_loaded(env, CJniCall::GetObjectField(env, pdcl, "pathList", "Ldalvik/system/DexPathList;").l);
    if(!pathList_loaded.GetObj())
        return FALSE;

    CJniObj dexElements_loaded(env, CJniCall::GetObjectField(env, pathList_loaded, "dexElements", "[Ldalvik/system/DexPathList$Element;").l);
    if(!dexElements_loaded.GetObj())
        return FALSE;

    CJniObj pathList_loading(env, CJniCall::GetObjectField(env, dcl, "pathList", "Ldalvik/system/DexPathList;").l);
    if(!pathList_loading.GetObj())
        return FALSE;

    CJniObj dexElements_loading(env, CJniCall::GetObjectField(env, pathList_loading, "dexElements", "[Ldalvik/system/DexPathList$Element;").l);
    if(!dexElements_loading.GetObj())
        return FALSE;

    CJniObj elementClass(env, env->GetObjectClass(dexElements_loaded));
    CJniObj componentType(env, CJniCall::CallObjectMethod(env, elementClass, "getComponentType", "()Ljava/lang/Class;").l);
    if(!componentType.GetObj())
        return FALSE;

    jsize len_loaded = env->GetArrayLength(dexElements_loaded);
    jsize len_new = len_loaded + 1;
    CJniObj dexElements_new(env, CJniCall::CallClassMethod(env, "java/lang/reflect/Array", "newInstance", "(Ljava/lang/Class;I)Ljava/lang/Object;", componentType.GetObj(), len_new).l);
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
    CJniCall::SetObjectField(env, pathList_loaded, "dexElements", "[Ldalvik/system/DexPathList$Element;", value);
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
    if(!CAppContext::IsMainProcess(env))
        return TRUE;//����Ǻ�̨���̣����ﲻ��Ҫ�Ż�����Ϊ�������Ѿ��Ż�����

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
        //6.0���Ժ�汾�������������
        cmdline += " --instruction-set-variant=" + string(variant);
    }
    if(g_sdkver >= Oreo_8_1)
    {
        //8.1���Ժ�汾�������������
        cmdline += " --class-loader-context=\"&\"";
    }
    char buf[1024];
    cmdline += " 2>&1";
    //���Ż�һ�Σ���ֹapp�� --compiler-filter=verify-none --compiler-filter=interpret-only����,���·��������ɻ�����
    FILE *pf = popen(cmdline.c_str(), "r");
    size_t once = fread(buf, 1, sizeof(buf), pf);
    pclose(pf);
    return !once;
}
