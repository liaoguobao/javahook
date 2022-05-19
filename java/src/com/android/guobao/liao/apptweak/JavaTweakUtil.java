package com.android.guobao.liao.apptweak;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.Arrays;
import android.app.Application;

public class JavaTweakUtil {
    static public String hexToString(byte[] hex, boolean upper) {
        if (hex == null || hex.length <= 0) {
            return "";
        }
        byte[] digit = upper ? "0123456789ABCDEF".getBytes() : "0123456789abcdef".getBytes();
        byte[] str = new byte[hex.length << 1];

        for (int i = 0; i < hex.length; i++) {
            int cb = hex[i] & 0xFF; //byte范围为[-128, 127]，如果要表示为[0, 255]，必须要用int来表示，且要和0xff进行与操作后再赋值，否则在负数情况下后续查表会出现数组越界
            str[(i << 1) + 0] = digit[cb >> 4];
            str[(i << 1) + 1] = digit[cb & 0x0F];
        }
        String str_ = new String(str);
        return str_;
    }

    static public byte[] stringToHex(String str) {
        if (str == null || str.length() <= 0 || (str.length() & 1) != 0) {
            return null;
        }
        byte[] str_ = str.getBytes();
        byte[] hex = new byte[str_.length >> 1];

        for (int i = 0; i < str_.length; i += 2) {
            byte ch = str_[i + 0];
            byte cl = str_[i + 1];

            byte cb = 0;
            if (ch >= '0' && ch <= '9')
                cb |= ch - '0';
            else if (ch >= 'a' && ch <= 'f')
                cb |= ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F')
                cb |= ch - 'A' + 10;
            else
                return null;

            cb <<= 4;
            if (cl >= '0' && cl <= '9')
                cb |= cl - '0';
            else if (cl >= 'a' && cl <= 'f')
                cb |= cl - 'a' + 10;
            else if (cl >= 'A' && cl <= 'F')
                cb |= cl - 'A' + 10;
            else
                return null;

            hex[i >> 1] = cb;
        }
        return hex;
    }

    static public String hexToVisible(byte[] hex, boolean upper) {
        if (hex == null || hex.length <= 0) {
            return "";
        }
        int len_ = 0;
        byte[] digit = upper ? "0123456789ABCDEF".getBytes() : "0123456789abcdef".getBytes();
        byte[] str = new byte[hex.length * 3];

        for (int i = 0; i < hex.length; i++) {
            int cb = hex[i] & 0xFF; //byte范围为[-128, 127]，如果要表示为[0, 255]，必须要用int来表示，且要和0xff进行与操作后再赋值，否则在负数情况下后续查表会出现数组越界
            if (cb > 0x20 && cb < 0x7F) {
                str[len_++] = (byte) cb;
            } else {
                str[len_++] = ' ';
                str[len_++] = digit[cb >> 4];
                str[len_++] = digit[cb & 0x0F];
            }
        }
        String str_ = new String(str, 0, len_);
        return str_;
    }

    static public byte[] visibleToHex(String str) {
        if (str == null || str.length() <= 0) {
            return null;
        }
        int len_ = 0;
        byte[] str_ = str.getBytes();
        byte[] hex = new byte[str_.length];

        for (int i = 0; i < str_.length; i++) {
            if (str_[i] != ' ') {
                hex[len_++] = str_[i];
            } else if (i + 2 >= str_.length) {
                return null;
            } else {
                byte ch = str_[i + 1];
                byte cl = str_[i + 2];
                i += 2;

                byte cb = 0;
                if (ch >= '0' && ch <= '9')
                    cb |= ch - '0';
                else if (ch >= 'a' && ch <= 'f')
                    cb |= ch - 'a' + 10;
                else if (ch >= 'A' && ch <= 'F')
                    cb |= ch - 'A' + 10;
                else
                    return null;

                cb <<= 4;
                if (cl >= '0' && cl <= '9')
                    cb |= cl - '0';
                else if (cl >= 'a' && cl <= 'f')
                    cb |= cl - 'a' + 10;
                else if (cl >= 'A' && cl <= 'F')
                    cb |= cl - 'A' + 10;
                else
                    return null;

                hex[len_++] = cb;
            }
        }
        hex = Arrays.copyOf(hex, len_);
        return hex;
    }

    static public byte[] bufferToByte(ByteBuffer buf) {
        if (!buf.hasRemaining()) {
            return null;
        }
        if (buf.hasArray()) {
            int ofs = buf.arrayOffset();
            byte[] ba = Arrays.copyOfRange(buf.array(), ofs + buf.position(), ofs + buf.limit());
            return ba;
        } else {
            byte[] ba = new byte[buf.remaining()];
            buf.get(ba, 0, ba.length);
            return ba;
        }
    }

    static public String getFieldNameByType(Class<?> clazz, String type, int index) {
        int pos = 0;
        Field[] fs = clazz.getDeclaredFields();
        for (int i = 0; i < fs.length; i++) {
            String gt = fs[i].getGenericType().toString();
            int spec = gt.lastIndexOf(' ');
            gt = (spec == -1 ? gt : gt.substring(spec + 1));
            if (gt.equals(type) && pos++ == index) {
                return fs[i].getName();
            }
        }
        return null;
    }

    static public Field getDeclaredFieldEx(Class<?> c, String name, String type, int index) {
        try {
            Field f = c.getDeclaredField(name);
            return f;
        } catch (Exception e) {
            name = getFieldNameByType(c, type, index);
        }
        if (name == null) {
            return null;
        }
        try {
            Field f = c.getDeclaredField(name);
            return f;
        } catch (Exception e2) {
            return null;
        }
    }

    static public String getMethodNameBySign(Class<?> clazz, String sign, int index) {
        int pos = 0;
        Method[] ms = clazz.getDeclaredMethods();
        for (int i = 0; i < ms.length; i++) {
            String decl = ms[i].toGenericString();
            decl = decl.substring(0, decl.lastIndexOf(')') + 1);
            String retn = decl.substring(0, decl.lastIndexOf(' '));
            retn = retn.substring(retn.lastIndexOf(' ') + 1);
            decl = decl.substring(decl.lastIndexOf('('));
            decl += retn;
            if (decl.equals(sign) && pos++ == index) {
                return ms[i].getName();
            }
        }
        return null;
    }

    static public Method getDeclaredMethodEx(Class<?> c, String method, String sign, int index) {
        Method m = findClassMethod(c, method);
        if (m != null) {
            return m;
        }
        String name = getMethodNameBySign(c, sign, index);
        if (name == null) {
            return null;
        }
        method = name + sign.substring(0, sign.lastIndexOf(')') + 1);
        m = findClassMethod(c, method);
        return m;
    }

    static public Object getObjectField(Object o, String name) {
        try {
            Field f = o.getClass().getDeclaredField(name);
            f.setAccessible(true);
            Object v = f.get(o);
            return v;
        } catch (Exception e) {
            return null;
        }
    }

    static public boolean setObjectField(Object o, String name, Object v) {
        try {
            Field f = o.getClass().getDeclaredField(name);
            f.setAccessible(true);
            f.set(o, v);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    static public Object getClassField(Class<?> c, String name) {
        try {
            Field f = c.getDeclaredField(name);
            f.setAccessible(true);
            Object v = f.get(null);
            return v;
        } catch (Exception e) {
            return null;
        }
    }

    static public boolean setClassField(Class<?> c, String name, Object v) {
        try {
            Field f = c.getDeclaredField(name);
            f.setAccessible(true);
            f.set(null, v);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    static public Method findClassMethod(Class<?> clazz, String method) {
        boolean isname = (method.indexOf('(') == -1);
        if (!isname) {
            method = clazz.getName() + "." + method;
        }
        Method[] ms = clazz.getDeclaredMethods();
        for (int i = 0; i < ms.length; i++) {
            String decl = (!isname ? ms[i].toString() : ms[i].getName());
            if (!isname) {
                decl = decl.substring(0, decl.lastIndexOf(')') + 1);
                decl = decl.substring(decl.lastIndexOf(' ') + 1);
            }
            if (decl.equals(method)) {
                return ms[i];
            }
        }
        return null;
    }

    static public Object callObjectMethod(Object o, String method, Object... args) {
        try {
            Method m = findClassMethod(o.getClass(), method);
            m.setAccessible(true);
            Object hr = m.invoke(o, args);
            return hr;
        } catch (Exception e) {
            return null;
        }
    }

    static public Object callClassMethod(Class<?> c, String method, Object... args) {
        try {
            Method m = findClassMethod(c, method);
            m.setAccessible(true);
            Object hr = m.invoke(null, args);
            return hr;
        } catch (Exception e) {
            return null;
        }
    }

    static public String currentPackageName() {
        try {
            Class<?> ActivityThread = Class.forName("android.app.ActivityThread");
            String s = (String) callClassMethod(ActivityThread, "currentPackageName()");
            return s;
        } catch (Exception e) {
            return null;
        }
    }

    static public String currentProcessName() {
        try {
            Class<?> ActivityThread = Class.forName("android.app.ActivityThread");
            String s = (String) callClassMethod(ActivityThread, "currentProcessName()");
            return s;
        } catch (Exception e) {
            return null;
        }
    }

    static public Application currentApplication() {
        try {
            Class<?> ActivityThread = Class.forName("android.app.ActivityThread");
            Application s = (Application) callClassMethod(ActivityThread, "currentApplication()");
            return s;
        } catch (Exception e) {
            return null;
        }
    }
}
