//HookHelper.cpp

#include "HookHelper.h"
#include "hook_trampoline_arm.h"

#include "../include/JniUtil.h"
#include "../include/LinkerUtil.h"
#include "../ApiHook/ApiHook.h"
//#include "../include/DebugUtil.h"

static unsigned     g_offsetCompiledCode = 0;//>=7.0[24], 12.0[20]
static unsigned     g_offsetAccessFlags = 0; //>=7.0[4]

//[>=11]jfieldID JniIdManager::EncodeFieldId(ArtField* field);
static jfieldID  new_ART_JniIdManager_EncodeFieldId (void *thiz, void *field);
static jfieldID(*old_ART_JniIdManager_EncodeFieldId)(void *thiz, void *field) = 0;

//[>=11]jmethodID JniIdManager::EncodeMethodId(ArtMethod* method);
static jmethodID  new_ART_JniIdManager_EncodeMethodId (void *thiz, void *method);
static jmethodID(*old_ART_JniIdManager_EncodeMethodId)(void *thiz, void *method) = 0;

//[==9]template Action GetMemberActionImpl<ArtField>(ArtField* member, HiddenApiAccessFlags::ApiList api_list, Action action, AccessMethod access_method);
static int  new_ART_HiddenApi_GetMemberActionImpl_ArtField (void *member, int api_list, int access_method);
static int(*old_ART_HiddenApi_GetMemberActionImpl_ArtField)(void *member, int api_list, int access_method) = 0;

//[==9]template Action GetMemberActionImpl<ArtMethod>(ArtMethod* member, HiddenApiAccessFlags::ApiList api_list, Action action, AccessMethod access_method);
static int  new_ART_HiddenApi_GetMemberActionImpl_ArtMethod (void *member, int api_list, int access_method);
static int(*old_ART_HiddenApi_GetMemberActionImpl_ArtMethod)(void *member, int api_list, int access_method) = 0;

//[>=10]template bool ShouldDenyAccessToMemberImpl<ArtField>(ArtField* member, ApiList api_list, AccessMethod access_method);
static char  new_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField (void *member, int api_list, int access_method);
static char(*old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField)(void *member, int api_list, int access_method) = 0;

//[>=10]template bool ShouldDenyAccessToMemberImpl<ArtMethod>(ArtMethod* member, ApiList api_list, AccessMethod access_method);
static char  new_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod (void *member, int api_list, int access_method);
static char(*old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod)(void *member, int api_list, int access_method) = 0;

//[<=5.1]void Instrumentation::UpdateMethodsCode(mirror::ArtMethod* method, const void* quick_code, const void* portable_code, bool have_portable_code);
//[==6.0]void Instrumentation::UpdateMethodsCode(        ArtMethod* method, const void* quick_code);
//[>=7.0]void Instrumentation::UpdateMethodsCodeImpl(    ArtMethod* method, const void* quick_code);
static void  new_ART_Instrumentation_UpdateMethodsCodeImpl (void *thiz, void *method, void *quick_code);
static void(*old_ART_Instrumentation_UpdateMethodsCodeImpl)(void *thiz, void *method, void *quick_code) = 0;

CHookHelper::CHookHelper()
{
}

CHookHelper::~CHookHelper()
{
}

int CHookHelper::InitHookHelper()
{
    int sdkver = CJniUtil::GetAndroidSdkVer();
    if(sdkver < Pie_9_0)
    {
        return 0;
    }
    void *libart = CLinkerUtil::dlopen(string(CJniUtil::GetAndroidVmDir()+string("libart.so")).c_str(), 0);
    if(1)
    {
        g_offsetAccessFlags = 4;
        g_offsetCompiledCode = (sdkver<=AndroidR_11_0 ? (POINTER_ROUND_UP(20)+POINTER_SIZE) : (POINTER_ROUND_UP(16)+POINTER_SIZE));
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art15instrumentation15Instrumentation21UpdateMethodsCodeImplEPNS_9ArtMethodEPKv"), (size_t)new_ART_Instrumentation_UpdateMethodsCodeImpl, (size_t *)&old_ART_Instrumentation_UpdateMethodsCodeImpl);
    }
    if(sdkver == Pie_9_0)
    {
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art9hiddenapi6detail19GetMemberActionImplINS_8ArtFieldEEENS0_6ActionEPT_NS_20HiddenApiAccessFlags7ApiListES4_NS0_12AccessMethodE"), (size_t)new_ART_HiddenApi_GetMemberActionImpl_ArtField, (size_t *)&old_ART_HiddenApi_GetMemberActionImpl_ArtField);
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art9hiddenapi6detail19GetMemberActionImplINS_9ArtMethodEEENS0_6ActionEPT_NS_20HiddenApiAccessFlags7ApiListES4_NS0_12AccessMethodE"), (size_t)new_ART_HiddenApi_GetMemberActionImpl_ArtMethod, (size_t *)&old_ART_HiddenApi_GetMemberActionImpl_ArtMethod);
    }
    else if(sdkver >= AndroidQ_10_0)
    {
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art9hiddenapi6detail28ShouldDenyAccessToMemberImplINS_8ArtFieldEEEbPT_NS0_7ApiListENS0_12AccessMethodE"), (size_t)new_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField, (size_t *)&old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField);
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art9hiddenapi6detail28ShouldDenyAccessToMemberImplINS_9ArtMethodEEEbPT_NS0_7ApiListENS0_12AccessMethodE"), (size_t)new_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod, (size_t *)&old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod);
    }
    if(sdkver >= AndroidR_11_0)
    {
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art3jni12JniIdManager13EncodeFieldIdEPNS_8ArtFieldE"), (size_t)new_ART_JniIdManager_EncodeFieldId, (size_t *)&old_ART_JniIdManager_EncodeFieldId);
        CApiHook::HookFunction((size_t)dlsym(libart, "_ZN3art3jni12JniIdManager14EncodeMethodIdEPNS_9ArtMethodE"), (size_t)new_ART_JniIdManager_EncodeMethodId, (size_t *)&old_ART_JniIdManager_EncodeMethodId);
    }
    //CDebugUtil::WriteToLogcat("sdkver=%d, EncodeFieldId=%p, EncodeMethodId=%p, GetMemberActionImpl_ArtField=%p, GetMemberActionImpl_ArtMethod=%p, ShouldDenyAccessToMemberImpl_ArtField=%p, ShouldDenyAccessToMemberImpl_ArtMethod=%p, UpdateMethodsCodeImpl=%p\r\n", sdkver, old_ART_JniIdManager_EncodeFieldId, old_ART_JniIdManager_EncodeMethodId, old_ART_HiddenApi_GetMemberActionImpl_ArtField, old_ART_HiddenApi_GetMemberActionImpl_ArtMethod, old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField, old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod, old_ART_Instrumentation_UpdateMethodsCodeImpl);
    return 0;
}

static jfieldID  new_ART_JniIdManager_EncodeFieldId (void *thiz, void *field)
{
    //jfieldID hr = old_ART_JniIdManager_EncodeFieldId(thiz, field);
    return (jfieldID)field;
}

static jmethodID  new_ART_JniIdManager_EncodeMethodId (void *thiz, void *method)
{
    //jmethodID hr = old_ART_JniIdManager_EncodeMethodId(thiz, method);
    return (jmethodID)method;
}

static int  new_ART_HiddenApi_GetMemberActionImpl_ArtField (void *member, int api_list, int access_method)
{
    //int hr = old_ART_HiddenApi_GetMemberActionImpl_ArtField(member, api_list, access_method);
    return 0;
}

static int  new_ART_HiddenApi_GetMemberActionImpl_ArtMethod (void *member, int api_list, int access_method)
{
    //int hr = old_ART_HiddenApi_GetMemberActionImpl_ArtMethod(member, api_list, access_method);
    return 0;
}

static char  new_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField (void *member, int api_list, int access_method)
{
    //char hr = old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtField(member, api_list, access_method);
    return 0;
}

static char  new_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod (void *member, int api_list, int access_method)
{
    //char hr = old_ART_HiddenApi_ShouldDenyAccessToMemberImpl_ArtMethod(member, api_list, access_method);
    return 0;
}

static void  new_ART_Instrumentation_UpdateMethodsCodeImpl (void *thiz, void *method, void *quick_code)
{
    //10及以前的版本，ClassStatus::kInitialized(14)状态是类的最终状态，java层Class.forName调用后，类的静态方法都已经调用了FixupStaticTrampolines函数
    //11及以后的版本，ClassStatus::kVisiblyInitialized(15)状态才是最终状态，java层Class.forName调用后，类的状态只能到ClassStatus::kInitialized(14)
    //此时静态方法还没有调用FixupStaticTrampolines函数，当类中的静态方法被hook后，执行类方法时FixupStaticTrampolines函数被调用，
    //函数内部会调用Instrumentation::UpdateMethodsCodeImpl函数更新入口点指针，这里做一次拦截，如果已经是hook的方法则强制不让更新入口点
    int ishook = !!(*(unsigned *)((char *)method + g_offsetAccessFlags) & kAccTweakHookedMethod);
    quick_code = ishook ? *(void **)((char *)method + g_offsetCompiledCode) : quick_code;

    old_ART_Instrumentation_UpdateMethodsCodeImpl(thiz, method, quick_code);
}
