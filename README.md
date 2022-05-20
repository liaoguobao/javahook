1、javatweak框架目前支持从5.0到11.0的所有版本（无12系统的手机，未测试）,不支持native、interface、abstract等方法的hook

2、demo目录为测试用例，如果不想编译demo，在demo\so______loader\bin\目录下已经编译好了现成的两种架构的apk包。

3、测试步骤

    adb install demo\so______loader\bin\so______loader.armeabi.apk
    
    adb shell mkdir /sdcard/tweak/com.example.so______loader
    
    adb push java\javatweak.dex /sdcard/tweak/com.example.so______loader
    
    一定要为app开启读写手机存储卡的权限，即可以读写/sdcard目录
    现在可以点开测试app，点击其中的按钮查看测试结果【注意测试日志需要在logcat中查看】
    com.example.so______loader为测试包的包名，如果在你自己的app中测试，这个要换成你的app包名
    
4、java目录为javatweak.dex插件文件生成目录，此目录主要用于编写hook代码，如果更新了hook代码内容，可以通过java2dex.bat脚本重新生成。

5、native目录是libjavahook.so的源码目录，如果更改了源码可以自己重新编译生成此so。例子apk中，附带了编译好的so

