// LinkerUtil.h

#ifndef _LINKER_UTIL_H_
#define _LINKER_UTIL_H_

#include "ProcMaps.h"
#include "ElfUtil.h"
#include "FileUtil.h"

class CLinkerUtil
{
protected:
    CLinkerUtil()
    {
    }
    ~CLinkerUtil()
    {
    }
public:
    static void* dlopen(const char *filename, int flag)
    {
        static void* (*do_dlopen)(const char *filename, int flag, const void* extinfo, const void* caller_addr) = 0;

        int sdkver = 0;
        if(!do_dlopen)
        {
            char sdk[PROP_VALUE_MAX];
            __system_property_get("ro.build.version.sdk", sdk);
            sdkver = atoi(sdk);
        }
        if(sdkver>0 && sdkver<=23)
        {
            *(void **)&do_dlopen = (void *)-1;
        }
        if(sdkver>=24)
        {
            string elf;
            if(!CFileUtil::ReadBinDataFromFile("/system/bin/linker", elf))
                return 0;

            size_t base; string path;
            if(!CProcMaps::GetModulePathAndBase("linker", path, base))
                return 0;

            const char *symbol = 0;
            if(sdkver==24 || sdkver==25)//7.0\7.1
                symbol = "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv"; //void* do_dlopen(const char* filename, int flags, const android_dlextinfo* extinfo,       void* caller_addr)
            else//>=8.0
                symbol = "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv";//void* do_dlopen(const char* filename, int flags, const android_dlextinfo* extinfo, const void* caller_addr)

            unsigned offset_do_dlopen = CElfUtil::GetSym_offset((unsigned char *)elf.c_str(), symbol);
            if(!offset_do_dlopen)
                return 0;

            *(void **)&do_dlopen = (void *)(base + offset_do_dlopen);
        }
        if((void *)do_dlopen == (void *)-1)
        {
            //6.0及以下版本，对dlopen打开动态库没有任何限制，直接调用原始方法
            return ::dlopen(filename, flag);
        }
        if(do_dlopen)
        {
            //7.0及以上版本会进到这里，注意第四个参数设置了open函数的地址，告诉linker,此次调用是从libc中发起，从而绕过dlopen打开动态库的限制，
            //dlopen默认情况下不可以加载/system/lib目录下的尚未加载的动态库，对已加载的/system/lib下的动态库除了/etc/public.libraries.txt文件指定的外，其它动态库在调用dlsym函数时都是返回0
            void *handle = do_dlopen(filename, flag, 0, (void *)::open);
            if(handle)//7.0\7.1\8.0\8.1会在这里提前返回
                return handle;

            size_t base; string path;
            const char *n = strrchr(filename, '/');
            CProcMaps::GetModulePathAndBase(n?n+1:filename, path, base);

            //9.0及以上版本会进到这里，注意第四个参数设置了模块本身的基地址，告诉linker,此次调用是动态库自身发起，从而绕过dlopen打开动态库的限制，
            //dlopen默认情况下不可以加载/system/lib目录下的尚未加载的动态库，对已加载的/system/lib下的动态库除了/etc/public.libraries.txt文件指定的外，其它动态库在调用dlsym函数时都是返回0
            return do_dlopen(filename, flag, 0, base?(void *)base:(void *)::open);
        }
        return 0;
    }
    static void* dlsym(const char *filename, const char *symbol, int isclose = 1)
    {
        void *h = CLinkerUtil::dlopen(filename, 0);
        void *s = h ? ::dlsym(h, symbol) : 0;
        if(isclose && h) ::dlclose(h);
        return s;
    }
};

#endif
