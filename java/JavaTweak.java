package com.android.guobao.liao.apptweak;

import com.android.guobao.liao.apptweak.JavaTweakBridge;

import android.util.Log;

public class JavaTweak {
    static protected void defineJavaClass(ClassLoader loader, Class<?> clazz) {
        JavaTweak_demo.defineJavaClass(loader, clazz);
    }
}

@SuppressWarnings("unused")
class JavaTweak_demo {
	static boolean hooked1 = false;
	static boolean hooked2 = false;
    static protected void defineJavaClass(ClassLoader loader, Class<?> clazz) {
        String name = clazz.getName();
        //Log.i("WriteToLogcat", "defineJavaClass: "+ name);
        //�����hook������javatweak.dex���غ�ż��أ����Բ������������жϷ�ʽhook����
        if (name.equals("com.example.so______loader.MainActivity")) {
            JavaTweakBridge.hookJavaMethod(loader, name, "onCreate");
            return;
        }
        //�����hook������javatweak.dex����ǰ���Ѿ����أ����Բ������������жϷ�ʽhook����
        if (!hooked1 && JavaTweakBridge.hookJavaMethod("android.telephony.TelephonyManager", "getDeviceId")) {
            hooked1 = true;
        }
        //�����hook�ķ��������������ط�����hookʱ������ʾָ��������������������ط����������ʡ�Բ�������
        if (!hooked2 && JavaTweakBridge.hookJavaMethod("android.widget.Toast", "makeText(android.content.Context,java.lang.CharSequence,int)")) {
            hooked2 = true;
        }
    }

    //static
    static private Object makeText(Object context, CharSequence text, int duration) {
    	text = String.format("makeText<%s>", text);
        return JavaTweakBridge.callStaticOriginalMethod(context, text, duration);
    }
    //instance
    static private void onCreate(Object thiz, Object savedInstanceState) {
        JavaTweakBridge.callOriginalMethod(thiz, savedInstanceState);
    }
    static private String getDeviceId(Object thiz) {
    	String s = JavaTweakBridge.callOriginalMethod(thiz);
    	s = String.format("110-%s", s);
    	return s;
    }
}
