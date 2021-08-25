# javahook

                                                      ART虚拟机中Java方法Hook的实现

目前主流的JAVA层hook框架是xposed，但需要重编译art虚拟机，也过于笨重，更重要的是会被app各种检测，有些app一旦发现安装直接强制退出。
为了不依赖xposed实现Java方法Hook，深入了解art运行机制，因而重新实现了一个轻型的Hook框架，目前已经在Andorid 5.0到8.1的系统上测试通过。
在实现的过程中，参考了不少同行的实现思路（legend、SandHook、YAHFA），借鉴了其中做的比较好的地方再结合了自己的一些理解，力求在使用上尽可能的简单，设计上尽可能降低系统版本依赖。

详细介绍请参考【安卓ART虚拟机Java Hook框架JavaTweak的设计与实现.docx】


目前支持从5.0到10.0的所有版本
