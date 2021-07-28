package com.example.so______loader;

import android.app.Activity;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends Activity {
	private Button mBtnStatic;
	private Button mBtnInstance;
	private MainActivity mThisActivity;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		mThisActivity = this;
		mBtnStatic = (Button) findViewById(R.id.btn_staticMethod);
		mBtnInstance = (Button) findViewById(R.id.btn_instanceMethod);
		
		mBtnStatic.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Toast.makeText(mThisActivity, "Hello world!", Toast.LENGTH_SHORT).show();
			}
		});
	
		mBtnInstance.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				//请为手机开放获取手机识别码权限，否则调用会崩溃
				String s = ((TelephonyManager) getSystemService(TELEPHONY_SERVICE)).getDeviceId();
				Toast.makeText(mThisActivity, s, Toast.LENGTH_SHORT).show();
			}
		});
	}
}
