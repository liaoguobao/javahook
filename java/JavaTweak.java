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
        //如果需hook的类在javatweak.dex加载后才加载，可以采用下面这种判断方式hook方法
        if (name.equals("com.example.so______loader.MainActivity")) {
            JavaTweakBridge.hookJavaMethod(loader, name, "onCreate");
            return;
        }
        //如果需hook的类在javatweak.dex加载前就已经加载，可以采用下面这种判断方式hook方法
        if (!hooked1) {
            hooked1 = true;
            JavaTweakBridge.hookJavaMethod("android.telephony.TelephonyManager", "getDeviceId");
        }
        //如果需hook的方法在类中有重载方法，hook时必须显示指定方法参数，如果无重载方法，则可以省略参数声明
        if (!hooked2) {
            hooked2 = true;
            JavaTweakBridge.hookJavaMethod("android.widget.Toast", "makeText(android.content.Context,java.lang.CharSequence,int)");
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
