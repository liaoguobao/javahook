::ִ�б��ű���jdk�汾Ϊ1.8.0��dx�汾Ϊ30.0.2
::���jdk��1.8���߸��߰汾��Ҫ�����Ͱ汾�ֽ���ʱ������ʾָ���汾�ţ���-source -target ����ͬʱʹ��,������Ϊ��ͬ�İ汾��ֵ��-source 1.7 -target 1.7��
::���dx�İ汾�ϵͶ�javac�汾�ϸ�ʱ��dx��������dex�Ĺ����п��ܻ���ְ汾��ͻ����bad class file magic (cafebabe) or version (0034.0000)��
::����23.0.1�汾��dx�����ֻ֧�ֵ�jdk 1.7,���jdk��1.8����dx������23�汾���ϣ�����javac����ʱָ������1.7�汾���ֽ���



@echo off
set ANDROID_API=%ANDROID_SDK%\platforms\android-24
mkdir %~dp0\tmp
javac -bootclasspath %ANDROID_API%\android.jar -classpath %ANDROID_API%\optional\*.jar -d %~dp0\tmp %~dp0\src\com\android\guobao\liao\apptweak\*.java 
jar -cf %~dp0\tmp\javatweak.jar -C %~dp0\tmp .
cmd /c "dx --dex --output=%~dp0\javatweak.dex %~dp0\tmp\javatweak.jar"
rmdir /s /q %~dp0\tmp


