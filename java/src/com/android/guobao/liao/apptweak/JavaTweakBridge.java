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

    static private native void nativePrintLog(int bufid, int prio, String tag, String msg);

    static public void writeToLogcat(int prio, String msg) {
        nativePrintLog(0, prio, "WriteToLogcat", msg);
    }

    static public void writeToLogcat(int prio, String format, Object... args) {
        writeToLogcat(prio, String.format(format, args));
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

    static public boolean hookJavaMethod(Class<?> tweak_class, String tweak_method, String hook_method) {
        boolean hr = hookJavaMethod(tweak_class.getClassLoader(), tweak_class.getName(), tweak_method, null, hook_method);
        return hr;
    }

    static public boolean hookJavaMethod(Class<?> tweak_class, String tweak_method) {
        boolean hr = hookJavaMethod(tweak_class.getClassLoader(), tweak_class.getName(), tweak_method, null, null);
        return hr;
    }

    static public boolean hookJavaMethod(String tweak_class, String tweak_method, String hook_method) {
        boolean hr = hookJavaMethod(null, tweak_class, tweak_method, null, hook_method);
        return hr;
    }

    static public boolean hookJavaMethod(String tweak_class, String tweak_method) {
        boolean hr = hookJavaMethod(null, tweak_class, tweak_method, null, null);
        return hr;
    }

    static public boolean hookJavaMethod(ClassLoader tweak_loader, String tweak_class, String tweak_method, String hook_method) {
        boolean hr = hookJavaMethod(tweak_loader, tweak_class, tweak_method, null, hook_method);
        return hr;
    }

    static public boolean hookJavaMethod(ClassLoader tweak_loader, String tweak_class, String tweak_method) {
        boolean hr = hookJavaMethod(tweak_loader, tweak_class, tweak_method, null, null);
        return hr;
    }

    static private boolean hookJavaMethod(ClassLoader tweak_loader, String tweak_class, String tweak_method, String hook_class, String hook_method) {
        try {
            Class<?> tweak_class_ = null;
            Class<?> hook_class_ = null;
            String hook_method_name = null;

            if (tweak_loader == null) {
                tweak_class_ = Class.forName(tweak_class);
            } else {
                tweak_class_ = Class.forName(tweak_class, true, tweak_loader);
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
                hook_method_name = (index == -1 ? tweak_method : tweak_method.substring(0, index));
                hook_method_name = (index == 0 ? tweak_class_.getSimpleName() : hook_method_name);
                hook_method = hook_method_name;
            } else {
                int index = hook_method.indexOf('(');
                hook_method_name = (index == -1 ? hook_method : hook_method.substring(0, index));
            }
            Method m = nativeHookMethod(tweak_class_, tweak_method, hook_class_, hook_method);
            if (m == null) {
                boolean hooked = backupMethods.containsKey(hook_method_name);
                writeToLogcat(hooked ? Log.WARN : Log.ERROR, "hookJavaMethod: method<%s> hook %s.", tweak_method, hooked ? "repeat" : "error");
                return false;
            }
            backupMethods.put(hook_method_name, m);
            writeToLogcat(Log.INFO, "hookJavaMethod: method<%s> hook ok.", tweak_method);
            return true;
        } catch (Throwable e) {
            writeToLogcat(Log.ERROR, e.toString());
            return false;
        }
    }

    @SuppressWarnings("unchecked")
    static private <T> T callOriginalMethod(boolean log, String name, Object receiver, Object... args) {
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
            if (log) {
                writeToLogcat(Log.INFO, paramsToString(name, m, hr, receiver, args));
            }
            return hr;
        } catch (Throwable e) {
            writeToLogcat(Log.ERROR, "callOriginalMethod: name<%s> error<%s>.", name, e);
            return null;
        }
    }

    static public <T> T callStaticOriginalMethod(Object... args) {
        T hr = callOriginalMethod(true, null, null, args);
        return hr;
    }

    static public <T> T callOriginalMethod(Object receiver, Object... args) {
        T hr = callOriginalMethod(true, null, receiver, args);
        return hr;
    }

    static public <T> T nologStaticOriginalMethod(Object... args) {
        T hr = callOriginalMethod(false, null, null, args);
        return hr;
    }

    static public <T> T nologOriginalMethod(Object receiver, Object... args) {
        T hr = callOriginalMethod(false, null, receiver, args);
        return hr;
    }

    static private String paramsToString(String name, Method m, Object hr, Object receiver, Object... args) {
        final int maxlen = 4096;
        Class<?> type = m.getReturnType();
        Class<?>[] types = m.getParameterTypes();

        String log = String.format("%s::%s%s->{\r\n", m.getDeclaringClass().getName(), m.getName(), !m.getName().equals(name) ? "@" + name : "");
        log += String.format("\t_this_ = %s->%s\r\n", m.getDeclaringClass().getName(), receiver);
        for (int i = 0; i < args.length; i++) {
            String byteArr = ((args[i] instanceof byte[]) ? JavaTweakUtil.hexToVisible(((byte[]) args[i]).length > maxlen ? Arrays.copyOf((byte[]) args[i], maxlen) : (byte[]) args[i], true) : null);
            String objArr = ((args[i] instanceof Object[]) ? Arrays.deepToString((Object[]) args[i]) : null);
            log += String.format("\tparam%d = %s->%s\r\n", i + 1, types[i].getName(), byteArr != null ? byteArr : (objArr != null ? objArr : args[i]));
        }
        String byteArr = ((hr instanceof byte[]) ? JavaTweakUtil.hexToVisible(((byte[]) hr).length > maxlen ? Arrays.copyOf((byte[]) hr, maxlen) : (byte[]) hr, true) : null);
        String objArr = ((hr instanceof Object[]) ? Arrays.deepToString((Object[]) hr) : null);

        log += String.format("\treturn = %s->%s\r\n}\r\n", type.getName(), byteArr != null ? byteArr : (objArr != null ? objArr : hr));
        return log;
    }
}
