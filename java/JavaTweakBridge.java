package com.android.guobao.liao.apptweak;

import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import android.util.Log;

public class JavaTweakBridge {
    /*
     * �����滻����������ͳһǰ׺{com.android.guobao.liao.apptweak.JavaTweak_***}
     */
    static private final String hookClassPrefix = JavaTweakBridge.class.getPackage().getName() + ".JavaTweak_";
    /*
     * key: hook�����ķ�����
     * value: hook�����ı��ݷ���
     */
    static private final ConcurrentHashMap<String, Method> backupMethods = new ConcurrentHashMap<String, Method>();
    /*
     * name: ��Ҫ���ص�����
     */
    static private native Class<?> nativeLoadClass(String name);

    static private Class<?> loadJavaClass(ClassLoader loader, String name) {
        Class<?> clazz = null;
        try {
            if (loader == null) {
                clazz = Class.forName(name);
            } else {
                clazz = Class.forName(name, true, loader);
            }
        } catch (Throwable e) {
            if (clazz == null) {
                clazz = nativeLoadClass(name);
            }
            if (clazz == null) {
                Log.e("WriteToLogcat", String.format("hookJavaMethod: loader<%s>, name<%s> forName error<%s>.", loader, name, e));
            }
        }
        return clazz;
    }
    /*
     * tweak_class: ���滻������������
     * hook_class: �滻������������
     * tweak_method�����滻������: onCreate(android.os.Bundle,int,java.lang.Object[])������ֻд������
     * hook_method���滻������: onCreate(java.lang.Object,android.os.Bundle,int,java.lang.Object[])������ֻд������
     * ���滻���������������Ļ��������������뾫ȷ���壬�����пո񣬲��������֮���ö��ŷָ�
     * �滻���������������Ļ���������������ģ�����壨�ǻ������Ϳ���ֱ����java.lang.Object���棬�򻯺������壬���������������뱻�滻��������һ�£��������пո񣬲��������֮���ö��ŷָ�
     * ���滻��������ǷǾ�̬����ʱ���滻�����ĵ�һ�����������Ǳ��滻������thisָ�룬�����������κ��ƣ����滻�������һ��this������
     * �滻���������Ǿ�̬����
     * return: ����ֵΪ���滻�����ı��ݷ���,��������ԭ����
     */
    static private native Method nativeHookMethod(Class<?> tweak_class, String tweak_method, Class<?> hook_class, String hook_method);

    static public boolean hookJavaMethod(ClassLoader tweak_loader, String tweak_class, String tweak_method) {
        boolean hr = hookJavaMethod(tweak_loader, tweak_class, tweak_method, null, null);
        return hr;
    }

    static public boolean hookJavaMethod(String tweak_class, String tweak_method) {
        boolean hr = hookJavaMethod(null, tweak_class, tweak_method, null, null);
        return hr;
    }

    static private boolean hookJavaMethod(ClassLoader tweak_loader, String tweak_class, String tweak_method, String hook_class, String hook_method) {
        try {
            Class<?> tweak_class_ = null;
            Class<?> hook_class_ = null;
            String hook_method_name = null;

            tweak_class_ = loadJavaClass(tweak_loader, tweak_class);
            if (tweak_class_ == null) {
                return false;
            }
            if (hook_class == null) {
                StackTraceElement[] stes = new Throwable().getStackTrace();
                //stes[0].getClassName()=="com.android.guobao.liao.apptweak.JavaTweakBridge", skip it.
                for (int i = 1; i < stes.length; i++) {
                    if (stes[i].getClassName().startsWith(hookClassPrefix)) {
                        hook_class_ = Class.forName(stes[i].getClassName());
                        break;
                    }
                }
            } else {
                hook_class_ = Class.forName(hook_class);
            }
            if (hook_method == null) {
                int index = tweak_method.indexOf('(');
                hook_method_name = (index==-1 ? tweak_method : tweak_method.substring(0, index));
                hook_method = hook_method_name;
            } else {
                int index = hook_method.indexOf('(');
                hook_method_name = (index==-1 ? hook_method : hook_method.substring(0, index));
            }
            Method m = nativeHookMethod(tweak_class_, tweak_method, hook_class_, hook_method);
            if (m == null) {
                boolean hooked = backupMethods.containsKey(hook_method_name);
                Log.println(hooked?Log.WARN:Log.ERROR, "WriteToLogcat", String.format("hookJavaMethod: method<%s> hook %s.", tweak_method, hooked?"repeat":"error"));
                return false;
            }
            backupMethods.put(hook_method_name, m);
            Log.i("WriteToLogcat", String.format("hookJavaMethod: method<%s> hook ok.", tweak_method));
            return true;
        } catch (Throwable e) {
            Log.e("WriteToLogcat", e.toString());
            return false;
        }
    }

    @SuppressWarnings("unchecked")
    static private <T> T callOriginalMethod(String name, Object receiver, Object... args) {
        try {
            if (name == null) {
                StackTraceElement[] stes = new Throwable().getStackTrace();
                //stes[0].getClassName()=="com.android.guobao.liao.apptweak.JavaTweakBridge", skip it.
                //stes[0].getMethodName()=="callOriginalMethod", skip it.
                for (int i = 1; i < stes.length; i++) {
                    if (stes[i].getClassName().startsWith(hookClassPrefix)) {
                        name = stes[i].getMethodName();
                        break;
                    }
                }
            }
            Method m = backupMethods.get(name);
            T hr = (T) m.invoke(receiver, args);

            printCallParams(m, hr, args);
            return hr;
        } catch (Throwable e) {
            Log.e("WriteToLogcat", String.format("callOriginalMethod: name<%s> error<%s>.", name, e));
            return null;
        }
    }

    static public <T> T callStaticOriginalMethod(Object... args) {
        T hr = callOriginalMethod(null, null, args);
        return hr;
    }

    static public <T> T callOriginalMethod(Object receiver, Object... args) {
        T hr = callOriginalMethod(null, receiver, args);
        return hr;
    }

    static private void printCallParams(Method m, Object hr, Object... args) {
        Class<?> type = m.getReturnType();
        Class<?>[] types = m.getParameterTypes();

        String log = String.format("%s::%s->{\r\n", m.getDeclaringClass().getName(), m.getName());
        for (int i = 0; i < args.length; i++) {
            String byteArr = ((args[i] instanceof   byte[]) ? new String         ((  byte[])args[i]) : null);
            String objArr  = ((args[i] instanceof Object[]) ? Arrays.deepToString((Object[])args[i]) : null);
            log += String.format("\tparam%d = %s->%s\r\n", i+1, types[i].getName(), byteArr!=null?byteArr:(objArr!=null?objArr:args[i]));
        }
        String byteArr = ((hr instanceof   byte[]) ? new String         ((  byte[])hr) : null);
        String objArr  = ((hr instanceof Object[]) ? Arrays.deepToString((Object[])hr) : null);
        log += String.format("\treturn = %s->%s\r\n}\r\n", type.getName(), byteArr!=null?byteArr:(objArr!=null?objArr:hr));
        Log.i("WriteToLogcat", log);
    }
}
