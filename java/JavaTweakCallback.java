package com.android.guobao.liao.apptweak;

public class JavaTweakCallback {
    @SuppressWarnings("unused")
    static private void defineJavaClass(ClassLoader loader, Class<?> clazz) {
        JavaTweak.defineJavaClass(loader, clazz);
    }
}
