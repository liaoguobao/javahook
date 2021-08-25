// JniUtil.h

#ifndef _JNI_UTIL_H_
#define _JNI_UTIL_H_

#include "platform.h"

#ifdef __ANDROID__

#define JAVA_ARRAY_NULL     0
#define JAVA_ARRAY_BOOL     (JAVA_ARRAY_NULL + 1)
#define JAVA_ARRAY_BYTE     (JAVA_ARRAY_BOOL + 1)
#define JAVA_ARRAY_CHAR     (JAVA_ARRAY_BYTE + 1)
#define JAVA_ARRAY_SHORT    (JAVA_ARRAY_CHAR + 1)
#define JAVA_ARRAY_INT      (JAVA_ARRAY_SHORT + 1)
#define JAVA_ARRAY_LONG     (JAVA_ARRAY_INT + 1)
#define JAVA_ARRAY_FLOAT    (JAVA_ARRAY_LONG + 1)
#define JAVA_ARRAY_DOUBLE   (JAVA_ARRAY_FLOAT + 1)
#define JAVA_ARRAY_OBJECT   (JAVA_ARRAY_DOUBLE + 1)
#define JAVA_ARRAY_STRING   (JAVA_ARRAY_OBJECT + 1)

class CJniObj
{
protected:
    JNIEnv *m_env;
    jobject m_obj;
    int     m_global;

public:
    CJniObj(JNIEnv *env = 0, jobject obj = 0, int global = 0)
    {
        m_env = env;
        m_obj = obj;
        m_global = global;
    }
    CJniObj(const CJniObj &jo)
    {
        m_env = jo.m_env;
        m_obj = jo.m_obj;
        m_global = jo.m_global;

        if(!m_env || !m_obj)
            return;
        if(!m_global)
            m_obj = m_env->NewLocalRef(m_obj);
        if(m_global)
            m_obj = m_env->NewGlobalRef(m_obj);
    }
    ~CJniObj()
    {
        Release();
    }

public:
    CJniObj &operator=(const CJniObj &jo)
    {
        if(this != &jo)
        {
            Attach(jo.m_env, jo.m_obj, jo.m_global, 1);
        }
        return *this;
    }
    inline operator jobject() const
    {
        return (jobject)GetObj();
    }
    inline operator jclass() const
    {
        return (jclass)GetObj();
    }
    inline operator jstring() const
    {
        return (jstring)GetObj();
    }
    inline operator jarray() const
    {
        return (jarray)GetObj();
    }
    inline operator jobjectArray() const
    {
        return (jobjectArray)GetObj();
    }
    inline operator jbooleanArray() const
    {
        return (jbooleanArray)GetObj();
    }
    inline operator jbyteArray() const
    {
        return (jbyteArray)GetObj();
    }
    inline operator jcharArray() const
    {
        return (jcharArray)GetObj();
    }
    inline operator jshortArray() const
    {
        return (jshortArray)GetObj();
    }
    inline operator jintArray() const
    {
        return (jintArray)GetObj();
    }
    inline operator jlongArray() const
    {
        return (jlongArray)GetObj();
    }
    inline operator jfloatArray() const
    {
        return (jfloatArray)GetObj();
    }
    inline operator jdoubleArray() const
    {
        return (jdoubleArray)GetObj();
    }
    inline operator jthrowable() const
    {
        return (jthrowable)GetObj();
    }

public:
    inline jobject GetObj() const
    {
        return m_obj;
    }
    void Attach(JNIEnv *env, jobject obj, int global = 0, int newobj = 0)
    {
        Release();

        m_env = env;
        m_obj = obj;
        m_global = global;

        if(!newobj)
            return;
        if(!m_env || !m_obj)
            return;
        if(!m_global)
            m_obj = m_env->NewLocalRef(m_obj);
        if(m_global)
            m_obj = m_env->NewGlobalRef(m_obj);
    }
    jobject Detach()
    {
        jobject obj = m_obj;

        m_obj = 0;
        m_env = 0;
        m_global = 0;
        return obj;
    }
    void Release()
    {
        if(m_env && m_obj && !m_global)
            m_env->DeleteLocalRef(m_obj);
        if(m_env && m_obj && m_global)
            m_env->DeleteGlobalRef(m_obj);
        if(m_env)
            m_env->ExceptionClear();

        Detach();
    }
};

class CJavaToBase
{
protected:
    void      * jv;
    JNIEnv    * env;
    jobject     ja;
    jsize       len;
    int         type;
    string      str;

public:
    CJavaToBase(JNIEnv *e, jbooleanArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_BOOL;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetBooleanArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jboolean{", len);
            str = buf;

            for(jsize i = 0; i < len; ++i)
            {
                sprintf(buf, (i==len-1) ? "%s" : "%s, ", ((jboolean *)jv)[i]?"true":"false");
                str += buf;
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jbyteArray a, int isstring = 0)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_BYTE;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetByteArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jbyte{", len);
            str = buf;

            if(!isstring)
            {
                unsigned char cb;
                const char *digit = "0123456789ABCDEF";

                for(jsize i = 0; i < len; ++i)
                {
                    cb = ((unsigned char *)jv)[i];

                    if(cb > 0x20 && cb < 0x7F)
                    {
                        str += (char)cb;
                    }
                    else
                    {
                        str += ' ';
                        str += digit[cb >> 4];
                        str += digit[cb & 0x0F];
                    }
                }
            }
            else
            {
                str.append((char *)jv, (size_t)len);
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jcharArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_CHAR;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetCharArrayElements(a, 0);

            jstring _c = env->NewString((const jchar *)jv, len);
            const char *__c = env->GetStringUTFChars(_c, NULL);
            jsize _len = env->GetStringUTFLength(_c);

            char buf[32];
            sprintf(buf, "[%d]jchar{", len);
            str = buf;

            str.append(__c, (size_t)_len);

            str += "}";

            env->ReleaseStringUTFChars(_c, __c);
            env->DeleteLocalRef(_c);
        }
    }
    CJavaToBase(JNIEnv *e, jshortArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_SHORT;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetShortArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jshort{", len);
            str = buf;

            for(jsize i = 0; i < len; ++i)
            {
                sprintf(buf, (i==len-1) ? "0x%04x" : "0x%04x, ", ((unsigned short *)jv)[i]);
                str += buf;
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jintArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_INT;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetIntArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jint{", len);
            str = buf;

            for(jsize i = 0; i < len; ++i)
            {
                sprintf(buf, (i==len-1) ? "0x%08x" : "0x%08x, ", ((unsigned int *)jv)[i]);
                str += buf;
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jlongArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_LONG;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetLongArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jlong{", len);
            str = buf;

            for(jsize i = 0; i < len; ++i)
            {
                sprintf(buf, (i==len-1) ? "0x%016llx" : "0x%016llx, ", ((unsigned long long *)jv)[i]);
                str += buf;
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jfloatArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_FLOAT;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetFloatArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jfloat{", len);
            str = buf;

            for(jsize i = 0; i < len; ++i)
            {
                sprintf(buf, (i==len-1) ? "%.3f" : "%.3f, ", ((jfloat *)jv)[i]);
                str += buf;
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jdoubleArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_DOUBLE;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetArrayLength(a);
            jv = env->GetDoubleArrayElements(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jdouble{", len);
            str = buf;

            for(jsize i = 0; i < len; ++i)
            {
                sprintf(buf, (i==len-1) ? "%.3lf" : "%.3lf, ", ((jdouble *)jv)[i]);
                str += buf;
            }

            str += "}";
        }
    }
    CJavaToBase(JNIEnv *e, jstring a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_STRING;
        jv = NULL;
        len = 0;

        if(env && ja)
        {
            len = env->GetStringUTFLength(a);
            jv = (void *)env->GetStringUTFChars(a, 0);

            char buf[32];
            sprintf(buf, "[%d]jstring{", len);
            str = buf;

            str.append((const char *)jv, (size_t)len);

            str += "}";
        }
    }
    virtual ~CJavaToBase()
    {
        Release();
    }

public:
    void *GetValue() const
    {
        return jv;
    }
    int GetType() const
    {
        return type;
    }
    jsize GetLength() const
    {
        return len;
    }
    const char *ToString() const
    {
        return str.c_str();
    }
    inline operator const char *() const
    {
        return ToString();
    }
    int Release()
    {
        if(!jv)
            return 0;

        if(type == JAVA_ARRAY_BOOL)
        {
            env->ReleaseBooleanArrayElements((jbooleanArray)ja, (jboolean *)jv, 0);
        }
        else if(type == JAVA_ARRAY_BYTE)
        {
            env->ReleaseByteArrayElements((jbyteArray)ja, (jbyte *)jv, 0);
        }
        else if(type == JAVA_ARRAY_CHAR)
        {
            env->ReleaseCharArrayElements((jcharArray)ja, (jchar *)jv, 0);
        }
        else if(type == JAVA_ARRAY_SHORT)
        {
            env->ReleaseShortArrayElements((jshortArray)ja, (jshort *)jv, 0);
        }
        else if(type == JAVA_ARRAY_INT)
        {
            env->ReleaseIntArrayElements((jintArray)ja, (jint *)jv, 0);
        }
        else if(type == JAVA_ARRAY_LONG)
        {
            env->ReleaseLongArrayElements((jlongArray)ja, (jlong *)jv, 0);
        }
        else if(type == JAVA_ARRAY_FLOAT)
        {
            env->ReleaseFloatArrayElements((jfloatArray)ja, (jfloat *)jv, 0);
        }
        else if(type == JAVA_ARRAY_DOUBLE)
        {
            env->ReleaseDoubleArrayElements((jdoubleArray)ja, (jdouble *)jv, 0);
        }
        else if(type == JAVA_ARRAY_STRING)
        {
            env->ReleaseStringUTFChars((jstring)ja, (const char *)jv);
        }

        jv = 0;
        return 0;
    }
};

class CBaseToJava
{
protected:
    jobject     jv;
    JNIEnv    * env;
    jsize       len;
    int         type;

public:
    CBaseToJava(JNIEnv *e, const jboolean *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_BOOL;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewBooleanArray(len);
            env->SetBooleanArrayRegion((jbooleanArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jbyte *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_BYTE;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewByteArray(len);
            env->SetByteArrayRegion((jbyteArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jchar *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_CHAR;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewCharArray(len);
            env->SetCharArrayRegion((jcharArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jshort *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_SHORT;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewShortArray(len);
            env->SetShortArrayRegion((jshortArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jint *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_INT;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewIntArray(len);
            env->SetIntArrayRegion((jintArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jlong *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_LONG;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewLongArray(len);
            env->SetLongArrayRegion((jlongArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jfloat *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_FLOAT;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewFloatArray(len);
            env->SetFloatArrayRegion((jfloatArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const jdouble *b, jsize n)
    {
        env = e;
        len = n;
        jv = NULL;
        type = JAVA_ARRAY_DOUBLE;

        if(env && ((len && b) || (!len)))
        {
            jv = env->NewDoubleArray(len);
            env->SetDoubleArrayRegion((jdoubleArray)jv, 0, len, b);
        }
    }
    CBaseToJava(JNIEnv *e, const char *b, jsize n = -1)
    {
        env = e;
        jv = NULL;
        len = (n == -1 ? strlen(b) : n);
        type = JAVA_ARRAY_STRING;

        if(env && ((len && b) || (!len)))
        {
            char *_b = 0;

            if(n != -1)
            {
                _b = (char *)malloc(len + 1);
                memcpy(_b, b, len);
                _b[len] = 0;
                b = _b;
            }

            jv = env->NewStringUTF(b);

            if(n != -1)
            {
                free(_b);
            }
        }
    }
    virtual ~CBaseToJava()
    {
        Release();
    }

public:
    jobject GetObject() const
    {
        return jv;
    }
    int GetType() const
    {
        return type;
    }
    jsize GetLength() const
    {
        return len;
    }
    jobject Detach()
    {
        jobject _jv = jv;
        jv = NULL;
        return _jv;
    }
    int Release()
    {
        if(!jv)
            return 0;

        env->DeleteLocalRef(jv);
        jv = 0;
        return 0;
    }
};

class CJniUtil
{
protected:
    CJniUtil()
    {
    }
    ~CJniUtil()
    {
    }

public:
    static int GetAndroidSdkVer()
    {
        char sdk[PROP_VALUE_MAX];
        __system_property_get("ro.build.version.sdk", sdk);
        int sdkver = atoi(sdk);
        return sdkver;
    }
    static BOOL RegisterNative(JNIEnv *env, const char *clazz, const char *name, const char *sig, void *addr)
    {
        if(!env || !addr)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;
        if(!name || !*name)
            return FALSE;
        if(!sig || !*sig)
            return FALSE;

        CJniObj jc(env, env->FindClass(clazz));
        if(!jc.GetObj())
            return FALSE;

        JNINativeMethod nm;
        nm.fnPtr = addr;
        nm.name = name;
        nm.signature = sig;

        BOOL ok = !env->RegisterNatives(jc, &nm, 1);
        return ok;
    }
    static jclass ObjectToClass(JNIEnv *env, jobject obj, BOOL *isref = 0)
    {
        if(isref)
            *isref = FALSE;
        if(!env || !obj)
            return 0;

        if(!IsClassObject(env, obj))
        {
            //如果不是一个class对象，这里要调用一次obj.getClass获取到一个class对象
            jclass clazz = env->GetObjectClass(obj);
            return clazz;
        }
        else
        {
            //如果是一个class对象，这里直接用即可
            if(isref)
                *isref = TRUE;

            jclass clazz = (jclass)env->NewLocalRef(obj);
            return clazz;
        }
    }
    static jobject GetCurrentApplication(JNIEnv *env)
    {
        if(!env)
            return 0;

        CJniObj clsObj(env, CallClassMethod(env, "android/app/ActivityThread", "currentApplication", "()Landroid/app/Application;").l);
        if(!clsObj.GetObj())
            return 0;

        jobject obj = clsObj.Detach();
        return obj;
    }
    static BOOL GetObjectClassName(JNIEnv *env, jobject obj, string &name, BOOL real = 1)
    {
        name.clear();
        if(!env || !obj)
            return FALSE;

        BOOL isref;
        CJniObj clsObj(env, ObjectToClass(env, obj, &isref));
        if(!clsObj.GetObj())
            return FALSE;

        if(isref && !real)
            name = "java.lang.Class";//如果是一个类对象，直接赋值并返回true
        if(isref && !real)
            return TRUE;

        CJniObj strObj(env, CallObjectMethod(env, clsObj, "getName", "()Ljava/lang/String;").l);
        if(!strObj.GetObj())
            return FALSE;

        const char *jv = env->GetStringUTFChars(strObj, 0);
        name = jv;
        env->ReleaseStringUTFChars(strObj, jv);
        return TRUE;
    }
    static BOOL ObjectToString(JNIEnv *env, jobject obj, string &str)
    {
        str.clear();

        if(!env || !obj)
            return FALSE;

        GetObjectClassName(env, obj, str, 0);
        str += "->";

        if(str[0] == '[')
        {
            if(str[1] == 'Z')
            {
                CJavaToBase jtb(env, (jbooleanArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'B')
            {
                CJavaToBase jtb(env, (jbyteArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'C')
            {
                CJavaToBase jtb(env, (jcharArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'S')
            {
                CJavaToBase jtb(env, (jshortArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'I')
            {
                CJavaToBase jtb(env, (jintArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'J')
            {
                CJavaToBase jtb(env, (jlongArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'F')
            {
                CJavaToBase jtb(env, (jfloatArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'D')
            {
                CJavaToBase jtb(env, (jdoubleArray)obj);
                str += jtb.ToString();
            }
            else if(str[1] == 'L')
            {
                string oa;
                ObjectArrayToString(env, (jobjectArray)obj, oa);
                str = oa;
            }
        }
        else
        {
            CJniObj strObj(env, CallObjectMethod(env, obj, "toString", "()Ljava/lang/String;").l);
            if(!strObj.GetObj())
                return FALSE;

            const char *jv = env->GetStringUTFChars(strObj, 0);
            str += jv;
            env->ReleaseStringUTFChars(strObj, jv);
        }
        return TRUE;
    }
    static BOOL ObjectArrayToString(JNIEnv *env, jobjectArray arr, string &str)
    {
        str.clear();

        if(!env || !arr)
            return FALSE;

        string one;
        char buf[64];

        GetObjectClassName(env, arr, str, 0);
        str += "->";

        jsize len = env->GetArrayLength(arr);
        str += "{\r\n";

        for(jsize i = 0; i < len; ++i)
        {
            CJniObj obj(env, env->GetObjectArrayElement(arr, i));

            ObjectToString(env, obj, one);
            sprintf(buf, "\titem = %d, objstr = ", i);
            str += buf + one + "\r\n";
        }

        str += "}\r\n";
        return TRUE;
    }
    static JavaVM *GetJavaVM(int isArt = 1)
    {
        static jint (*JNI_GetCreatedJavaVMs)(JavaVM **, jsize, jsize *) = 0;
        if(!JNI_GetCreatedJavaVMs)
        {
            void *handle = (isArt?dlopen("libart.so", 0):dlopen("libdvm.so", 0));
            *(void **)&JNI_GetCreatedJavaVMs = dlsym(handle, "JNI_GetCreatedJavaVMs");
            dlclose(handle);
        }
        if(!JNI_GetCreatedJavaVMs)
            return 0;

        JavaVM *vm = 0;
        jsize vm_count = 0;
        JNI_GetCreatedJavaVMs(&vm, 1, &vm_count);
        return vm;
    }
    static BOOL GetClassMethods(JNIEnv *env, const char *clazz, vector<string> &methods)
    {
        methods.clear();
        if(!env)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;

        CJniObj clsObj(env, env->FindClass(clazz));
        if(!clsObj.GetObj())
            return FALSE;

        CJniObj arrObj(env, CallObjectMethod(env, clsObj, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;").l);
        if(!arrObj.GetObj())
            return FALSE;

        jsize len = env->GetArrayLength(arrObj);

        for(jsize i = 0; i < len; ++i)
        {
            CJniObj ele(env, env->GetObjectArrayElement(arrObj, i));

            string tostring;
            ObjectToString(env, ele, tostring);

            //java.lang.reflect.Method->public static java.lang.String com.bonree.agent.android.engine.external.JSONObjectInstrumentation.toString(org.json.JSONObject,int) throws org.json.JSONException
            const char *__beg = strstr(tostring.c_str(), "->");
            const char *__end = strrchr(__beg, ')');

            if(!__beg || !__end)
                continue;

            //public static java.lang.String com.bonree.agent.android.engine.external.JSONObjectInstrumentation.toString(org.json.JSONObject,int)
            string sign(__beg + 2, __end - __beg - 1);
            methods.push_back(sign);
        }
        return TRUE;
    }
    static BOOL GetClassMethods(JNIEnv *env, const char *clazz, string &str)
    {
        str.clear();
        if(!env)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;

        str = clazz;
        str += "->";

        vector<string> methods;
        GetClassMethods(env, clazz, methods);
        str += "{\r\n";

        for(size_t i = 0; i < methods.size(); ++i)
        {
            char buf[64];
            sprintf(buf, "\titem = %d, method = <", i);
            str += buf + methods[i] + ">\r\n";
        }

        str += "}\r\n";
        return TRUE;
    }
    static BOOL GetClassFields(JNIEnv *env, const char *clazz, vector<string> &fields)
    {
        fields.clear();
        if(!env)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;

        CJniObj clsObj(env, env->FindClass(clazz));
        if(!clsObj.GetObj())
            return FALSE;

        CJniObj arrObj(env, CallObjectMethod(env, clsObj, "getDeclaredFields", "()[Ljava/lang/reflect/Field;").l);
        if(!arrObj.GetObj())
            return FALSE;

        jsize len = env->GetArrayLength(arrObj);

        for(jsize i = 0; i < len; ++i)
        {
            CJniObj ele(env, env->GetObjectArrayElement(arrObj, i));

            string tostring;
            ObjectToString(env, ele, tostring);

            //java.lang.reflect.Field->private transient java.lang.Class java.lang.Object.shadow$_klass_
            const char *__beg = strstr(tostring.c_str(), "->");
            if(!__beg)
                continue;

            //private transient java.lang.Class java.lang.Object.shadow$_klass_
            string sign(__beg + 2);
            fields.push_back(sign);
        }
        return TRUE;
    }
    static BOOL GetClassFields(JNIEnv *env, const char *clazz, string &str)
    {
        str.clear();
        if(!env)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;

        str = clazz;
        str += "->";

        vector<string> fields;
        GetClassFields(env, clazz, fields);
        str += "{\r\n";

        for(size_t i = 0; i < fields.size(); ++i)
        {
            char buf[64];
            sprintf(buf, "\titem = %d, field = <", i);
            str += buf + fields[i] + ">\r\n";
        }

        str += "}\r\n";
        return TRUE;
    }
    static BOOL GetStackTraceString(JNIEnv *env, string &trace)
    {
        trace.clear();
        if(!env)
            return FALSE;

        CJniObj clsObj(env, NewClassObject(env, "java/lang/Throwable", 0));
        if(!clsObj.GetObj())
            return FALSE;

        CJniObj strObj(env, CallClassMethod(env, "android/util/Log", "getStackTraceString", "(Ljava/lang/Throwable;)Ljava/lang/String;", clsObj.GetObj()).l);
        if(!strObj.GetObj())
            return FALSE;

        const char *jv = env->GetStringUTFChars(strObj, 0);
        trace = jv;
        env->ReleaseStringUTFChars(strObj, jv);
        return TRUE;
    }
    static BOOL IsClassObject(JNIEnv *env, jobject obj)
    {
        if(!env || !obj)
            return FALSE;

        CJniObj clsCls(env, env->FindClass("java/lang/Class"));
        if(!clsCls.GetObj())
            return FALSE;

        BOOL b = env->IsInstanceOf(obj, clsCls);
        return b;
    }
    static jvalue CallClassMethod(JNIEnv *env, const char *clazz, const char *name, const char *sig, ...)
    {
        jvalue hr;
        hr.j = 0;
        if(!env)
            return hr;
        if(!clazz || !*clazz)
            return hr;
        if(!name || !*name)
            return hr;
        if(!sig || !*sig)
            return hr;

        CJniObj clsObj(env, env->FindClass(clazz)); //clazz: com/jar/test/hello
        if(!clsObj.GetObj())
            return hr;

        jmethodID methodID = env->GetStaticMethodID(clsObj, name, sig);
        if(!methodID)
            return hr;

        va_list args;
        va_start(args, sig);
        int flag = strchr(sig, ')')[1];

        if(flag == 'Z')
            hr.z = env->CallStaticBooleanMethodV(clsObj, methodID, args);
        else if(flag == 'B')
            hr.b = env->CallStaticByteMethodV(clsObj, methodID, args);
        else if(flag == 'C')
            hr.c = env->CallStaticCharMethodV(clsObj, methodID, args);
        else if(flag == 'S')
            hr.s = env->CallStaticShortMethodV(clsObj, methodID, args);
        else if(flag == 'I')
            hr.i = env->CallStaticIntMethodV(clsObj, methodID, args);
        else if(flag == 'J')
            hr.j = env->CallStaticLongMethodV(clsObj, methodID, args);
        else if(flag == 'F')
            hr.f = env->CallStaticFloatMethodV(clsObj, methodID, args);
        else if(flag == 'D')
            hr.d = env->CallStaticDoubleMethodV(clsObj, methodID, args);
        else if(flag == 'L' || flag == '[')
            hr.l = env->CallStaticObjectMethodV(clsObj, methodID, args);
        else if(flag == 'V')
            env->CallStaticVoidMethodV(clsObj, methodID, args);

        va_end(args);
        return hr;
    }
    static jvalue CallObjectMethod(JNIEnv *env, jobject obj, const char *name, const char *sig, ...)
    {
        jvalue hr;
        hr.j = 0;
        if(!env || !obj)
            return hr;
        if(!name || !*name)
            return hr;
        if(!sig || !*sig)
            return hr;

        CJniObj clsObj(env, env->GetObjectClass(obj));
        if(!clsObj.GetObj())
            return hr;

        jmethodID methodID = env->GetMethodID(clsObj, name, sig);
        if(!methodID)
            return hr;

        va_list args;
        va_start(args, sig);
        int flag = strchr(sig, ')')[1];

        if(flag == 'Z')
            hr.z = env->CallBooleanMethodV(obj, methodID, args);
        else if(flag == 'B')
            hr.b = env->CallByteMethodV(obj, methodID, args);
        else if(flag == 'C')
            hr.c = env->CallCharMethodV(obj, methodID, args);
        else if(flag == 'S')
            hr.s = env->CallShortMethodV(obj, methodID, args);
        else if(flag == 'I')
            hr.i = env->CallIntMethodV(obj, methodID, args);
        else if(flag == 'J')
            hr.j = env->CallLongMethodV(obj, methodID, args);
        else if(flag == 'F')
            hr.f = env->CallFloatMethodV(obj, methodID, args);
        else if(flag == 'D')
            hr.d = env->CallDoubleMethodV(obj, methodID, args);
        else if(flag == 'L' || flag == '[')
            hr.l = env->CallObjectMethodV(obj, methodID, args);
        else if(flag == 'V')
            env->CallVoidMethodV(obj, methodID, args);

        va_end(args);
        return hr;
    }
    static jvalue GetClassField(JNIEnv *env, const char *clazz, const char *name, const char *sig)
    {
        jvalue hr;
        hr.j = 0;
        if(!env)
            return hr;
        if(!clazz || !*clazz)
            return hr;
        if(!name || !*name)
            return hr;
        if(!sig || !*sig)
            return hr;

        CJniObj clsObj(env, env->FindClass(clazz));
        if(!clsObj.GetObj())
            return hr;

        jfieldID fieldID = env->GetStaticFieldID(clsObj, name, sig);
        if(!fieldID)
            return hr;

        int flag = sig[0];

        if(flag == 'Z')
            hr.z = env->GetStaticBooleanField(clsObj, fieldID);
        else if(flag == 'B')
            hr.b = env->GetStaticByteField(clsObj, fieldID);
        else if(flag == 'C')
            hr.c = env->GetStaticCharField(clsObj, fieldID);
        else if(flag == 'S')
            hr.s = env->GetStaticShortField(clsObj, fieldID);
        else if(flag == 'I')
            hr.i = env->GetStaticIntField(clsObj, fieldID);
        else if(flag == 'J')
            hr.j = env->GetStaticLongField(clsObj, fieldID);
        else if(flag == 'F')
            hr.f = env->GetStaticFloatField(clsObj, fieldID);
        else if(flag == 'D')
            hr.d = env->GetStaticDoubleField(clsObj, fieldID);
        else if(flag == 'L' || flag == '[')
            hr.l = env->GetStaticObjectField(clsObj, fieldID);

        return hr;
    }
    static jvalue GetObjectField(JNIEnv *env, jobject obj, const char *name, const char *sig)
    {
        jvalue hr;
        hr.j = 0;
        if(!env || !obj)
            return hr;
        if(!name || !*name)
            return hr;
        if(!sig || !*sig)
            return hr;

        CJniObj clsObj(env, env->GetObjectClass(obj));
        if(!clsObj.GetObj())
            return hr;

        jfieldID fieldID = env->GetFieldID(clsObj, name, sig);
        if(!fieldID)
            return hr;

        int flag = sig[0];

        if(flag == 'Z')
            hr.z = env->GetBooleanField(obj, fieldID);
        else if(flag == 'B')
            hr.b = env->GetByteField(obj, fieldID);
        else if(flag == 'C')
            hr.c = env->GetCharField(obj, fieldID);
        else if(flag == 'S')
            hr.s = env->GetShortField(obj, fieldID);
        else if(flag == 'I')
            hr.i = env->GetIntField(obj, fieldID);
        else if(flag == 'J')
            hr.j = env->GetLongField(obj, fieldID);
        else if(flag == 'F')
            hr.f = env->GetFloatField(obj, fieldID);
        else if(flag == 'D')
            hr.d = env->GetDoubleField(obj, fieldID);
        else if(flag == 'L' || flag == '[')
            hr.l = env->GetObjectField(obj, fieldID);

        return hr;
    }
    static BOOL SetClassField(JNIEnv *env, const char *clazz, const char *name, const char *sig, jvalue value)
    {
        if(!env)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;
        if(!name || !*name)
            return FALSE;
        if(!sig || !*sig)
            return FALSE;

        CJniObj clsObj(env, env->FindClass(clazz));
        if(!clsObj.GetObj())
            return FALSE;

        jfieldID fieldID = env->GetStaticFieldID(clsObj, name, sig);
        if(!fieldID)
            return FALSE;

        int flag = sig[0];

        if(flag == 'Z')
            env->SetStaticBooleanField(clsObj, fieldID, value.z);
        else if(flag == 'B')
            env->SetStaticByteField(clsObj, fieldID, value.b);
        else if(flag == 'C')
            env->SetStaticCharField(clsObj, fieldID, value.c);
        else if(flag == 'S')
            env->SetStaticShortField(clsObj, fieldID, value.s);
        else if(flag == 'I')
            env->SetStaticIntField(clsObj, fieldID, value.i);
        else if(flag == 'J')
            env->SetStaticLongField(clsObj, fieldID, value.j);
        else if(flag == 'F')
            env->SetStaticFloatField(clsObj, fieldID, value.f);
        else if(flag == 'D')
            env->SetStaticDoubleField(clsObj, fieldID, value.d);
        else if(flag == 'L' || flag == '[')
            env->SetStaticObjectField(clsObj, fieldID, value.l);

        return TRUE;
    }
    static BOOL SetObjectField(JNIEnv *env, jobject obj, const char *name, const char *sig, jvalue value)
    {
        if(!env || !obj)
            return FALSE;
        if(!name || !*name)
            return FALSE;
        if(!sig || !*sig)
            return FALSE;

        CJniObj clsObj(env, env->GetObjectClass(obj));
        if(!clsObj.GetObj())
            return FALSE;

        jfieldID fieldID = env->GetFieldID(clsObj, name, sig);
        if(!fieldID)
            return FALSE;

        int flag = sig[0];

        if(flag == 'Z')
            env->SetBooleanField(obj, fieldID, value.z);
        else if(flag == 'B')
            env->SetByteField(obj, fieldID, value.b);
        else if(flag == 'C')
            env->SetCharField(obj, fieldID, value.c);
        else if(flag == 'S')
            env->SetShortField(obj, fieldID, value.s);
        else if(flag == 'I')
            env->SetIntField(obj, fieldID, value.i);
        else if(flag == 'J')
            env->SetLongField(obj, fieldID, value.j);
        else if(flag == 'F')
            env->SetFloatField(obj, fieldID, value.f);
        else if(flag == 'D')
            env->SetDoubleField(obj, fieldID, value.d);
        else if(flag == 'L' || flag == '[')
            env->SetObjectField(obj, fieldID, value.l);

        return TRUE;
    }
    static jobject NewClassObject(JNIEnv *env, const char *clazz, const char *sig, ...)
    {
        if(!env)
            return 0;
        if(!clazz || !*clazz)
            return 0;
        if(!sig || !*sig)
            sig = "()V";

        CJniObj clsObj(env, env->FindClass(clazz));
        if(!clsObj.GetObj())
            return 0;

        jmethodID methodID = env->GetMethodID(clsObj, "<init>", sig);
        if(!methodID)
            return 0;

        va_list args;
        va_start(args, sig);
        jobject obj = env->NewObjectV(clsObj, methodID, args);
        va_end(args);
        return obj;
    }
    static void* ObjectToMirror(JNIEnv *env, jobject obj)
    {
        static void *  (*Thread_DecodeJObject)(void *thiz, jobject obj) = 0;
        if(!env || !obj)
            return 0;

        if(!Thread_DecodeJObject)
        {
            void *handle = dlopen("libart.so", 0);
            *(void **)&Thread_DecodeJObject = dlsym(handle, "_ZNK3art6Thread13DecodeJObjectEP8_jobject");
            dlclose(handle);
        }
        if(!Thread_DecodeJObject)
            return 0;

        void *mirror = Thread_DecodeJObject((void *)((unsigned *)env)[1], obj);
        return mirror;
    }
    static jobject MirrorToObject(JNIEnv *env, void *mirror)
    {
        static jobject (*JNIEnvExt_NewLocalRef)(void *thiz, void* mirror);
        if(!env || !mirror)
            return 0;

        if(!JNIEnvExt_NewLocalRef)
        {
            void *handle = dlopen("libart.so", 0);
            *(void **)&JNIEnvExt_NewLocalRef = dlsym(handle, "_ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE");
            dlclose(handle);
        }
        if(!JNIEnvExt_NewLocalRef)
            return 0;

        jobject obj = JNIEnvExt_NewLocalRef(env, mirror);
        return obj;
    }
    static BOOL GetPackageName(JNIEnv *env, string &pname)
    {
        pname.clear();
        if(!env)
            return FALSE;

        CJniObj app(env, GetCurrentApplication(env));
        if(!app.GetObj())
            return FALSE;

        CJniObj jname(env, CallObjectMethod(env, app, "getPackageName", "()Ljava/lang/String;").l);
        if(!jname.GetObj())
            return FALSE;

        const char *jv = env->GetStringUTFChars(jname, 0);
        pname = jv;
        env->ReleaseStringUTFChars(jname, jv);
        return TRUE;
    }
    static BOOL CreateAppDir(JNIEnv *env, const char *dname, string &dpath)
    {
        dpath.clear();
        if(!env)
            return FALSE;
        if(!dname || !*dname)
            return FALSE;

        CJniObj app(env, GetCurrentApplication(env));
        if(!app.GetObj())
            return FALSE;

        CBaseToJava jdname(env, dname);
        CJniObj jfile(env, CallObjectMethod(env, app, "getDir", "(Ljava/lang/String;I)Ljava/io/File;", jdname.GetObject(), 0).l);
        if(!jfile.GetObj())
            return FALSE;

        CJniObj jdpath(env, CallObjectMethod(env, jfile, "getAbsolutePath", "()Ljava/lang/String;").l);
        if(!jdpath.GetObj())
            return FALSE;

        const char *jv = env->GetStringUTFChars(jdpath, 0);
        dpath = jv;
        env->ReleaseStringUTFChars(jdpath, jv);
        return TRUE;
    }
    static jobjectArray NewObjectArray(JNIEnv *env, const char *clazz, int length, ...)
    {
        if(!env || !length)
            return 0;
        if(!clazz || !*clazz)
            return 0;

        CJniObj objCls(env, env->FindClass(clazz));
        if(!objCls.GetObj())
            return 0;

        CJniObj objArr(env, env->NewObjectArray(length, objCls, 0));
        if(!objArr.GetObj())
            return 0;

        va_list args;
        va_start(args, length);

        for(int i = 0; i < length; i++)
        {
            jobject obj = va_arg(args, jobject);
            env->SetObjectArrayElement(objArr, i, obj);
        }

        va_end(args);
        return (jobjectArray)objArr.Detach();
    }
    static jclass ClassForName(JNIEnv *env, const char *name, int init = 1, jobject loader = 0)
    {
        if(!env)
            return 0;
        if(!name || !*name)
            return 0;

        CJniObj loader_;
        CBaseToJava name_(env, name); //java.lang.String
        if(!loader)
        {
            jclass clazz = (jclass)CallClassMethod(env, "java/lang/Class", "forName", "(Ljava/lang/String;)Ljava/lang/Class;", name_.GetObject()).l;
            if(clazz)
                return clazz;

            CJniObj app(env, GetCurrentApplication(env));
            loader_.Attach(env, CallObjectMethod(env, app, "getClassLoader", "()Ljava/lang/ClassLoader;").l);
            loader = loader_;
        }
        jclass clazz = !loader?0:(jclass)CallClassMethod(env, "java/lang/Class", "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;", name_.GetObject(), init, loader).l;
        return clazz;
    }
};

class CJniEnv
{
protected:
    JNIEnv *m_env;
    JavaVM *m_vm;
    int     m_attach;

public:
    CJniEnv(JavaVM *vm = 0)
    {
        m_vm = vm;
        m_env = 0;
        m_attach = 0;

        if(!m_vm)
            m_vm = CJniUtil::GetJavaVM();
        if(m_vm)
            m_vm->GetEnv((void **)&m_env, JNI_VERSION_1_4);
        if(m_vm && !m_env)
            m_attach = 1;
        if(m_attach)
            m_vm->AttachCurrentThread(&m_env, 0);
    }
    ~CJniEnv()
    {
        Detach();
    }

public:
    JNIEnv *GetEnv() const
    {
        return m_env;
    }
    inline operator JNIEnv *() const
    {
        return GetEnv();
    }
    inline JNIEnv *operator->() const
    {
        return GetEnv();
    }
    JNIEnv *Detach()
    {
        if(m_attach)
        {
            m_env = 0;
            m_attach = 0;
            m_vm->DetachCurrentThread();
        }
        m_vm = 0;
        JNIEnv *env = m_env;
        m_env = 0;
        return env;
    }
};

#endif

#endif
