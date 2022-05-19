package com.example.so______loader;

import java.io.File;
import java.security.MessageDigest;

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import android.app.Activity;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {
    private Button mBtnLoad;
    private Button mBtnCheck;
    private Button mBtnDeCheck;
    private Button mBtnToken;
    private Button mBtnStart;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mBtnLoad = (Button) findViewById(R.id.btn_load);
        mBtnCheck = (Button) findViewById(R.id.btn_checkcode);
        mBtnDeCheck = (Button) findViewById(R.id.btn_decheckcode);
        mBtnToken = (Button) findViewById(R.id.btn_token);
        mBtnStart = (Button) findViewById(R.id.btn_start);

        mBtnLoad.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String dex = String.format("/sdcard/tweak/%s/javatweak.dex", getPackageName());
                if (new File(dex).canRead()) {
                    System.loadLibrary("javahook");
                } else {
                    Log.e("WriteToLogcat", dex + ": can not read");
                }
            }
        });

        mBtnCheck.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    byte[] bytes = "hello world".getBytes("UTF-8");
                    SecretKeySpec secretKeySpec = new SecretKeySpec("z66qa18l0w9o521k".getBytes(), "AES");
                    IvParameterSpec ivParameterSpec = new IvParameterSpec("16-Bytes--String".getBytes());
                    Cipher instance = Cipher.getInstance("AES/CBC/PKCS5Padding");
                    instance.init(1, secretKeySpec, ivParameterSpec);
                    byte[] enc = instance.doFinal(bytes);
                    String s = Base64.encodeToString(enc, Base64.NO_WRAP);
                    Log.i("WriteToLogcat", "aesenc: " + s);
                } catch (Exception e) {
                    Log.e("WriteToLogcat", e.toString());
                }
            }
        });

        mBtnDeCheck.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    byte[] decode = Base64.decode("RIRpoHgOVYeU2mkFlbp2TA==", Base64.NO_WRAP);
                    SecretKeySpec secretKeySpec = new SecretKeySpec("z66qa18l0w9o521k".getBytes(), "AES");
                    IvParameterSpec ivParameterSpec = new IvParameterSpec("16-Bytes--String".getBytes());
                    Cipher instance = Cipher.getInstance("AES/CBC/PKCS5Padding");
                    instance.init(2, secretKeySpec, ivParameterSpec);
                    byte[] dec = instance.doFinal(decode);
                    String s = new String(dec, "UTF-8");
                    Log.i("WriteToLogcat", "aesdec: " + s);
                } catch (Exception e) {
                    Log.e("WriteToLogcat", e.toString());
                }
            }
        });

        mBtnToken.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    MessageDigest md = MessageDigest.getInstance("MD5");
                    md.update("hello world".getBytes());
                    byte[] digest = md.digest();
                    String s = Base64.encodeToString(digest, Base64.NO_WRAP);
                    Log.i("WriteToLogcat", "md5: " + s);
                } catch (Exception e) {
                    Log.e("WriteToLogcat", e.toString());
                }
            }
        });

        mBtnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            }
        });
    }
}
