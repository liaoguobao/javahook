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

class CJniCall
{
protected:
    CJniCall()
    {
    }
    ~CJniCall()
    {
    }

public:
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
    static BOOL SetClassField(JNIEnv *env, const char *clazz, const char *name, const char *sig, jlong value)
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
            env->SetStaticBooleanField(clsObj, fieldID, (jboolean)value);
        else if(flag == 'B')
            env->SetStaticByteField(clsObj, fieldID, (jbyte)value);
        else if(flag == 'C')
            env->SetStaticCharField(clsObj, fieldID, (jchar)value);
        else if(flag == 'S')
            env->SetStaticShortField(clsObj, fieldID, (jshort)value);
        else if(flag == 'I')
            env->SetStaticIntField(clsObj, fieldID, (jint)value);
        else if(flag == 'J')
            env->SetStaticLongField(clsObj, fieldID, (jlong)value);
        else if(flag == 'F')
            env->SetStaticFloatField(clsObj, fieldID, (jfloat)value);
        else if(flag == 'D')
            env->SetStaticDoubleField(clsObj, fieldID, (jdouble)value);
        else if(flag == 'L' || flag == '[')
            env->SetStaticObjectField(clsObj, fieldID, (jobject)value);

        return TRUE;
    }
    static BOOL SetObjectField(JNIEnv *env, jobject obj, const char *name, const char *sig, jlong value)
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
            env->SetBooleanField(obj, fieldID, (jboolean)value);
        else if(flag == 'B')
            env->SetByteField(obj, fieldID, (jbyte)value);
        else if(flag == 'C')
            env->SetCharField(obj, fieldID, (jchar)value);
        else if(flag == 'S')
            env->SetShortField(obj, fieldID, (jshort)value);
        else if(flag == 'I')
            env->SetIntField(obj, fieldID, (jint)value);
        else if(flag == 'J')
            env->SetLongField(obj, fieldID, (jlong)value);
        else if(flag == 'F')
            env->SetFloatField(obj, fieldID, (jfloat)value);
        else if(flag == 'D')
            env->SetDoubleField(obj, fieldID, (jdouble)value);
        else if(flag == 'L' || flag == '[')
            env->SetObjectField(obj, fieldID, (jobject)value);

        return TRUE;
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
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetBooleanArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jbyteArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_BYTE;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetByteArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jcharArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_CHAR;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetCharArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jshortArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_SHORT;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetShortArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jintArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_INT;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetIntArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jlongArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_LONG;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetLongArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jfloatArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_FLOAT;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetFloatArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jdoubleArray a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_DOUBLE;
        len = (e&&a) ? env->GetArrayLength(a) : 0;
        jv = (e&&a) ? env->GetDoubleArrayElements(a, 0) : 0;
    }
    CJavaToBase(JNIEnv *e, jstring a)
    {
        env = e;
        ja = a;
        type = JAVA_ARRAY_STRING;
        len = (e&&a) ? env->GetStringUTFLength(a) : 0;
        jv = (e&&a) ? (char *)env->GetStringUTFChars(a, 0) : 0;
    }
    ~CJavaToBase()
    {
        Release();
    }

public:
    inline operator void*() const
    {
        return (void*)GetValue();
    }
    inline operator char*() const
    {
        return (char*)GetValue();
    }
    inline operator unsigned char*() const
    {
        return (unsigned char*)GetValue();
    }
    inline void *GetValue() const
    {
        return jv;
    }
    inline int GetType() const
    {
        return type;
    }
    inline jsize GetLength() const
    {
        return len;
    }
    int Release()
    {
        if(!jv)
        {
            return 0;
        }
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
        str.clear();
        return 0;
    }
    const char *ToString()
    {
        if(!jv || !str.empty())
        {
            return str.c_str();
        }
        if(type == JAVA_ARRAY_BOOL)
        {
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
        else if(type == JAVA_ARRAY_BYTE)
        {
            char buf[32];
            sprintf(buf, "[%d]jbyte{", len);

            str = buf;
            for(jsize i = 0; i < len; ++i)
            {
                unsigned char cb = ((unsigned char *)jv)[i];
                if(cb > 0x20 && cb < 0x7F)
                {
                    str += (char)cb;
                }
                else
                {
                    str += ' ';
                    str += "0123456789ABCDEF"[cb >> 4];
                    str += "0123456789ABCDEF"[cb & 0x0F];
                }
            }
            str += "}";
        }
        else if(type == JAVA_ARRAY_CHAR)
        {
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
        else if(type == JAVA_ARRAY_SHORT)
        {
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
        else if(type == JAVA_ARRAY_INT)
        {
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
        else if(type == JAVA_ARRAY_LONG)
        {
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
        else if(type == JAVA_ARRAY_FLOAT)
        {
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
        else if(type == JAVA_ARRAY_DOUBLE)
        {
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
        else if(type == JAVA_ARRAY_STRING)
        {
            char buf[32];
            sprintf(buf, "[%d]jstring{", len);

            str = buf;
            str.append((const char *)jv, (size_t)len);
            str += "}";
        }
        return str.c_str();
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
    ~CBaseToJava()
    {
        Release();
    }

public:
    inline operator jobject() const
    {
        return (jobject)GetObject();
    }
    inline operator jstring() const
    {
        return (jstring)GetObject();
    }
    inline operator jbyteArray() const
    {
        return (jbyteArray)GetObject();
    }
    inline jobject GetObject() const
    {
        return jv;
    }
    inline int GetType() const
    {
        return type;
    }
    inline jsize GetLength() const
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

class CAppContext
{
protected:
    CAppContext()
    {
    }
    ~CAppContext()
    {
    }

public:
    static BOOL GetPackageName(JNIEnv *env, string &pname)
    {
        pname.clear();
        if(!env)
            return FALSE;

        CJniObj jname(env, CJniCall::CallClassMethod(env, "android/app/ActivityThread", "currentPackageName", "()Ljava/lang/String;").l);
        if(!jname.GetObj())
            return FALSE;

        pname = CJavaToBase(env, (jstring)jname.GetObj());
        return TRUE;
    }
    static BOOL GetProcessName(JNIEnv *env, string &pname)
    {
        pname.clear();
        if(!env)
            return FALSE;

        CJniObj jname(env, CJniCall::CallClassMethod(env, "android/app/ActivityThread", "currentProcessName", "()Ljava/lang/String;").l);
        if(!jname.GetObj())
            return FALSE;

        pname = CJavaToBase(env, (jstring)jname.GetObj());
        return TRUE;
    }
    static BOOL IsMainProcess(JNIEnv *env)
    {
        string package, process;
        GetPackageName(env, package);
        GetProcessName(env, process);

        BOOL b = (!process.empty() && process==package);
        return b;
    }
    static jobject GetApplication(JNIEnv *env)
    {
        if(!env)
            return 0;

        CJniObj japp(env, CJniCall::CallClassMethod(env, "android/app/ActivityThread", "currentApplication", "()Landroid/app/Application;").l);
        if(!japp.GetObj())
            return 0;

        jobject app = japp.Detach();
        return app;
    }
    static jobject GetSystemContext(JNIEnv *env)
    {
        if(!env)
            return 0;

        CJniObj at(env, CJniCall::CallClassMethod(env, "android/app/ActivityThread", "currentActivityThread", "()Landroid/app/ActivityThread;").l);
        if(!at.GetObj())
            return 0;

        CJniObj ci(env, CJniCall::CallObjectMethod(env, at, "getSystemContext", "()Landroid/app/ContextImpl;").l);
        if(!ci.GetObj())
            return 0;

        jobject obj = ci.Detach();
        return obj;
    }
    static BOOL GetDataDir(JNIEnv *env, string &dpath)//data/user/0/com.example.so______loader
    {
        dpath.clear();
        if(!env)
            return FALSE;

        CJniObj ai(env, GetApplicationInfo(env));
        if(!ai.GetObj())
            return FALSE;

        CJniObj jdpath(env, CJniCall::GetObjectField(env, ai, "dataDir", "Ljava/lang/String;").l);
        if(!jdpath.GetObj())
            return FALSE;

        dpath = CJavaToBase(env, (jstring)jdpath.GetObj());
        return TRUE;
    }
    static BOOL CreateAppDir(JNIEnv *env, const char *dname, string &dpath)//data/user/0/com.example.so______loader/app_{dname}
    {
        dpath.clear();
        if(!env)
            return FALSE;
        if(!dname || !*dname)
            return FALSE;

        if(!GetDataDir(env, dpath))
            return FALSE;

        dpath += "/app_";
        dpath += dname;
        int hr = access(dpath.c_str(), 0);
        hr = (hr ? mkdir(dpath.c_str(), 0755) : hr);
        return !hr;
    }
    static jobject GetApplicationInfo(JNIEnv *env, int flags = 0)
    {
        if(!env)
            return FALSE;

        CJniObj context(env, GetSystemContext(env));
        if(!context.GetObj())
            return 0;

        CJniObj pm(env, CJniCall::CallObjectMethod(env, context, "getPackageManager", "()Landroid/content/pm/PackageManager;").l);
        if(!pm.GetObj())
            return 0;

        CJniObj jname(env, CJniCall::CallClassMethod(env, "android/app/ActivityThread", "currentPackageName", "()Ljava/lang/String;").l);
        if(!jname.GetObj())
            return 0;

        CJniObj jai(env, CJniCall::CallObjectMethod(env, pm, "getApplicationInfo", "(Ljava/lang/String;I)Landroid/content/pm/ApplicationInfo;", jname.GetObj(), flags).l);
        if(!jai.GetObj())
            return 0;

        jobject ai = jai.Detach();
        return ai;
    }
    static jobject GetPackageInfo(JNIEnv *env, int flags = 0)
    {
        if(!env)
            return 0;

        CJniObj context(env, GetSystemContext(env));
        if(!context.GetObj())
            return 0;

        CJniObj pm(env, CJniCall::CallObjectMethod(env, context, "getPackageManager", "()Landroid/content/pm/PackageManager;").l);
        if(!pm.GetObj())
            return 0;

        CJniObj jname(env, CJniCall::CallClassMethod(env, "android/app/ActivityThread", "currentPackageName", "()Ljava/lang/String;").l);
        if(!jname.GetObj())
            return 0;

        CJniObj jpi(env, CJniCall::CallObjectMethod(env, pm, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;", jname.GetObj(), flags).l);
        if(!jpi.GetObj())
            return 0;

        jobject pi = jpi.Detach();
        return pi;
    }
    static BOOL GetAppCert(JNIEnv *env, string &sign)
    {
        sign.clear();
        if(!env)
            return FALSE;

        CJniObj pi(env, GetPackageInfo(env, /*PackageManager.GET_SIGNATURES*/64));
        if(!pi.GetObj())
            return FALSE;

        CJniObj signatures(env, CJniCall::GetObjectField(env, pi, "signatures", "[Landroid/content/pm/Signature;").l);
        if(!signatures.GetObj())
            return FALSE;

        CJniObj signature(env, env->GetObjectArrayElement(signatures, 0));
        if(!signature.GetObj())
            return FALSE;

        CJniObj jsign(env, CJniCall::CallObjectMethod(env, signature, "toByteArray", "()[B").l);
        if(!jsign.GetObj())
            return FALSE;

        CJavaToBase sign_(env, (jbyteArray)jsign.GetObj());
        sign.assign((char *)sign_.GetValue(), sign_.GetLength());
        return TRUE;
    }
    static BOOL GetSourceDir(JNIEnv *env, string &dpath)//data/app/com.example.so______loader-G8MQWyEGdIQMNvVcXNWQbQ==/base.apk
    {
        dpath.clear();
        if(!env)
            return FALSE;

        CJniObj ai(env, GetApplicationInfo(env));
        if(!ai.GetObj())
            return FALSE;

        CJniObj jdpath(env, CJniCall::GetObjectField(env, ai, "sourceDir", "Ljava/lang/String;").l);
        if(!jdpath.GetObj())
            return FALSE;

        dpath = CJavaToBase(env, (jstring)jdpath.GetObj());
        return TRUE;
    }
    static BOOL GetLibraryDir(JNIEnv *env, string &dpath)//data/app/com.example.so______loader-G8MQWyEGdIQMNvVcXNWQbQ==/lib/arm
    {
        dpath.clear();
        if(!env)
            return FALSE;

        CJniObj ai(env, GetApplicationInfo(env));
        if(!ai.GetObj())
            return FALSE;

        CJniObj jdpath(env, CJniCall::GetObjectField(env, ai, "nativeLibraryDir", "Ljava/lang/String;").l);
        if(!jdpath.GetObj())
            return FALSE;

        dpath = CJavaToBase(env, (jstring)jdpath.GetObj());
        return TRUE;
    }
    static BOOL GetExternalStorageDir(JNIEnv *env, string &dpath)//storage/emulated/0
    {
        dpath.clear();
        if(!env)
            return FALSE;

        CJniObj esd(env, CJniCall::CallClassMethod(env, "android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;").l);
        if(!esd.GetObj())
            return 0;

        CJniObj jdpath(env, CJniCall::CallObjectMethod(env, esd, "getAbsolutePath", "()Ljava/lang/String;").l);
        if(!jdpath.GetObj())
            return FALSE;

        dpath = CJavaToBase(env, (jstring)jdpath.GetObj());
        return TRUE;
    }
    static BOOL GetAppCpuAbi(JNIEnv *env, string &abi)//armeabi,armeabi-v7a,arm64-v8a
    {
        abi.clear();
        if(!env)
            return FALSE;

        CJniObj ai(env, GetApplicationInfo(env));
        if(!ai.GetObj())
            return FALSE;

        CJniObj jabi(env, CJniCall::GetObjectField(env, ai, "primaryCpuAbi", "Ljava/lang/String;").l);
        if(!jabi.GetObj())
            return FALSE;

        abi = CJavaToBase(env, (jstring)jabi.GetObj());
        return TRUE;
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
    static BOOL Is64bit()
    {
        return (sizeof(void*) == 8);
    }
    static int GetAndroidSdkVer()
    {
        char sdk[PROP_VALUE_MAX];
        __system_property_get("ro.build.version.sdk", sdk);
        int sdkver = atoi(sdk);
        return sdkver;
    }
    static const char* GetAndroidVmDir()
    {
        if(GetAndroidSdkVer() >= 30) //AndroidR_11_0
        {
            return (Is64bit() ? "/apex/com.android.art/lib64/" : "/apex/com.android.art/lib/");
        }
        else if(GetAndroidSdkVer() >= 29) //AndroidQ_10_0
        {
            return (Is64bit() ? "/apex/com.android.runtime/lib64/" : "/apex/com.android.runtime/lib/");
        }
        else
        {
            return (Is64bit() ? "/system/lib64/" : "/system/lib/");
        }
    }
    static BOOL PtrToName(string &name, void *ptr = 0)
    {
        name.clear();
        Dl_info dli = {0};
        if(!dladdr(ptr?ptr:(void *)PtrToName, &dli) || !dli.dli_fname)
            return FALSE;

        name = dli.dli_fname;
        return TRUE;
    }
    static size_t PtrToBase(void *ptr = 0)
    {
        Dl_info dli = {0};
        if(!dladdr(ptr?ptr:(void *)PtrToBase, &dli) || !dli.dli_fbase)
            return 0;

        return (size_t)dli.dli_fbase;
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

        CJniObj strObj(env, CJniCall::CallObjectMethod(env, clsObj, "getName", "()Ljava/lang/String;").l);
        if(!strObj.GetObj())
            return FALSE;

        name = CJavaToBase(env, (jstring)strObj.GetObj());
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
            CJniObj strObj(env, CJniCall::CallObjectMethod(env, obj, "toString", "()Ljava/lang/String;").l);
            if(!strObj.GetObj())
                return FALSE;

            str += CJavaToBase(env, (jstring)strObj.GetObj());
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
    static int GetJavaVmOffset(JavaVM *vm = 0)
    {
        if(!vm)
            vm = GetJavaVM();
        if(!vm)
            return 0;

        size_t runtime = *(size_t *)(vm + 1);
        if(!runtime)
            return 0;

        for(int i = 0x100; i < 0x200; i += sizeof(size_t))
        {
            if(*(size_t *)(runtime + i) == (size_t)vm)
                return i;
        }
        return 0;
    }
    static BOOL GetStackTraceString(JNIEnv *env, string &trace)
    {
        trace.clear();
        if(!env)
            return FALSE;

        CJniObj clsObj(env, NewClassObject(env, "java/lang/Throwable", 0));
        if(!clsObj.GetObj())
            return FALSE;

        CJniObj strObj(env, CJniCall::CallClassMethod(env, "android/util/Log", "getStackTraceString", "(Ljava/lang/Throwable;)Ljava/lang/String;", clsObj.GetObj()).l);
        if(!strObj.GetObj())
            return FALSE;

        trace = CJavaToBase(env, (jstring)strObj.GetObj());
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

        void *mirror = Thread_DecodeJObject((void *)((size_t *)env)[1], obj);
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
            jclass clazz = (jclass)CJniCall::CallClassMethod(env, "java/lang/Class", "forName", "(Ljava/lang/String;)Ljava/lang/Class;", name_.GetObject()).l;
            if(clazz)
                return clazz;

            CJniObj context(env, CAppContext::GetSystemContext(env));
            loader_.Attach(env, CJniCall::CallObjectMethod(env, context, "getClassLoader", "()Ljava/lang/ClassLoader;").l);
            loader = loader_;
        }
        jclass clazz = !loader?0:(jclass)CJniCall::CallClassMethod(env, "java/lang/Class", "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;", name_.GetObject(), init, loader).l;
        return clazz;
    }
    static BOOL ClassToString(JNIEnv *env, const char *clazz, string &str)
    {
        str.clear();
        if(!env)
            return FALSE;
        if(!clazz || !*clazz)
            return FALSE;

        CJniObj clsObj(env, env->FindClass(clazz));
        if(!clsObj.GetObj())
            return FALSE;

        char buf[256];
        str = clazz;
        str += "->classToString{\r\n";

        CJniObj fields(env, CJniCall::CallObjectMethod(env, clsObj, "getDeclaredFields", "()[Ljava/lang/reflect/Field;").l);
        jsize flen = env->GetArrayLength(fields);
        for(jsize i = 0; i < flen; ++i)
        {
            CJniObj ele(env, env->GetObjectArrayElement(fields, i));
            CJniObj generic(env, CJniCall::CallObjectMethod(env, ele, "toGenericString", "()Ljava/lang/String;").l);

            //private transient java.lang.Class java.lang.Object.shadow$_klass_
            sprintf(buf, "\tf%02d: %s\r\n", i, (char *)CJavaToBase(env, (jstring)generic.GetObj()).GetValue());
            str += buf;
        }
        CJniObj constructors(env, CJniCall::CallObjectMethod(env, clsObj, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;").l);
        jsize clen = env->GetArrayLength(constructors);
        for(jsize i = 0; i < clen; ++i)
        {
            CJniObj ele(env, env->GetObjectArrayElement(constructors, i));
            CJniObj generic(env, CJniCall::CallObjectMethod(env, ele, "toGenericString", "()Ljava/lang/String;").l);

            //private java.lang.Object()
            sprintf(buf, "\tc%02d: %s\r\n", i, (char *)CJavaToBase(env, (jstring)generic.GetObj()).GetValue());
            str += buf;
        }
        CJniObj methods(env, CJniCall::CallObjectMethod(env, clsObj, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;").l);
        jsize mlen = env->GetArrayLength(methods);
        for(jsize i = 0; i < mlen; ++i)
        {
            CJniObj ele(env, env->GetObjectArrayElement(methods, i));
            CJniObj generic(env, CJniCall::CallObjectMethod(env, ele, "toGenericString", "()Ljava/lang/String;").l);

            //java.lang.reflect.Method->public static java.lang.String com.bonree.agent.android.engine.external.JSONObjectInstrumentation.toString(org.json.JSONObject,int) throws org.json.JSONException
            sprintf(buf, "\tm%02d: %s\r\n", i, (char *)CJavaToBase(env, (jstring)generic.GetObj()).GetValue());
            str += buf;
        }
        str += "}\r\n";
        return TRUE;
    }
    static BOOL FieldToString(JNIEnv *env, jobject obj, string &str)
    {
        str.clear();
        if(!env || !obj)
            return FALSE;

        CJniObj objClass(env, CJniUtil::ObjectToClass(env, obj));
        CJniObj objFields(env, CJniCall::CallObjectMethod(env, objClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;").l);

        CJniUtil::GetObjectClassName(env, obj, str);
        str += "->fieldToString{\r\n";

        jsize len = env->GetArrayLength(objFields);
        for(jsize i = 0; i < len; ++i)
        {
            CJniObj objField(env, env->GetObjectArrayElement(objFields, i));

            CJniObj fieldName(env, CJniCall::CallObjectMethod(env, objField, "getName", "()Ljava/lang/String;").l);
            CJavaToBase _fieldName(env, (jstring)fieldName.GetObj());

            CJniCall::CallObjectMethod(env, objField, "setAccessible", "(Z)V", 1);
            CJniObj fieldValue(env, CJniCall::CallObjectMethod(env, objField, "get", "(Ljava/lang/Object;)Ljava/lang/Object;", obj).l);

            string _fieldValue;
            CJniUtil::ObjectToString(env, fieldValue, _fieldValue);

            str += "\t";
            str += (char *)_fieldName.GetValue();
            str += " = " + _fieldValue + "\r\n";
        }
        str += "}\r\n";
        return TRUE;
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
    inline JNIEnv *GetEnv() const
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

class CApkUtil
{
protected:
    CApkUtil()
    {
    }
    ~CApkUtil()
    {
    }

public:
    static BOOL GetApkAsset(JNIEnv *env, const char *name, string &asset)
    {
        asset.clear();
        if(!env || !name || !*name)
            return FALSE;

        BOOL ok = 0;
        CJniObj zip, stream;
        do{
        string apk;
        if(!CAppContext::GetSourceDir(env, apk))
            break;

        zip.Attach(env, CJniUtil::NewClassObject(env, "java/util/zip/ZipFile", "(Ljava/lang/String;)V", CBaseToJava(env, apk.c_str()).GetObject()));
        if(!zip.GetObj())
            break;

        CJniObj entry(env, CJniCall::CallObjectMethod(env, zip, "getEntry", "(Ljava/lang/String;)Ljava/util/zip/ZipEntry;", CBaseToJava(env, name).GetObject()).l);
        if(!entry.GetObj())
            break;

        stream.Attach(env, CJniCall::CallObjectMethod(env, zip, "getInputStream", "(Ljava/util/zip/ZipEntry;)Ljava/io/InputStream;", entry.GetObj()).l);
        if(!stream.GetObj())
            break;

        int n = 0;
        jbyteArray buf = env->NewByteArray(0x1000);
        while((n = CJniCall::CallObjectMethod(env, stream, "read", "([B)I", buf).i) > 0)
        {
            char *d = (char *)env->GetByteArrayElements(buf, 0);
            asset.append(d, n);
            env->ReleaseByteArrayElements(buf, (jbyte *)d, 0);
        }
        env->DeleteLocalRef(buf);
        ok = 1;
        }while(0);
        CJniCall::CallObjectMethod(env, stream, "close", "()V");
        CJniCall::CallObjectMethod(env, zip, "close", "()V");
        return ok;
    }
    static BOOL ExtractApkAsset(JNIEnv *env, const char *name, const char *path)
    {
        if(!env || !name || !*name || !path || !*path)
            return FALSE;

        BOOL ok = 0;
        FILE *pf = 0;
        CJniObj zip, stream;
        do{
        pf = fopen(path, "wb");
        if(!pf)
            break;

        string apk;
        if(!CAppContext::GetSourceDir(env, apk))
            break;

        zip.Attach(env, CJniUtil::NewClassObject(env, "java/util/zip/ZipFile", "(Ljava/lang/String;)V", CBaseToJava(env, apk.c_str()).GetObject()));
        if(!zip.GetObj())
            break;

        CJniObj entry(env, CJniCall::CallObjectMethod(env, zip, "getEntry", "(Ljava/lang/String;)Ljava/util/zip/ZipEntry;", CBaseToJava(env, name).GetObject()).l);
        if(!entry.GetObj())
            break;

        stream.Attach(env, CJniCall::CallObjectMethod(env, zip, "getInputStream", "(Ljava/util/zip/ZipEntry;)Ljava/io/InputStream;", entry.GetObj()).l);
        if(!stream.GetObj())
            break;

        int n = 0;
        jbyteArray buf = env->NewByteArray(0x1000);
        while((n = CJniCall::CallObjectMethod(env, stream, "read", "([B)I", buf).i) > 0)
        {
            char *d = (char *)env->GetByteArrayElements(buf, 0);
            fwrite(d, n, 1, pf);
            env->ReleaseByteArrayElements(buf, (jbyte *)d, 0);
        }
        env->DeleteLocalRef(buf);
        ok = 1;
        }while(0);
        CJniCall::CallObjectMethod(env, stream, "close", "()V");
        CJniCall::CallObjectMethod(env, zip, "close", "()V");
        if(pf) fclose(pf);
        return ok;
    }
    static int ExistApkAsset(JNIEnv *env, const char *name)
    {
        if(!env || !name || !*name)
            return -1;

        string apk;
        if(!CAppContext::GetSourceDir(env, apk))
            return -1;

        CJniObj zip(env, CJniUtil::NewClassObject(env, "java/util/zip/ZipFile", "(Ljava/lang/String;)V", CBaseToJava(env, apk.c_str()).GetObject()));
        if(!zip.GetObj())
            return -1;

        CJniObj entry(env, CJniCall::CallObjectMethod(env, zip, "getEntry", "(Ljava/lang/String;)Ljava/util/zip/ZipEntry;", CBaseToJava(env, name).GetObject()).l);

        CJniCall::CallObjectMethod(env, zip, "close", "()V");
        return !!entry.GetObj();
    }
};
#endif
#endif
