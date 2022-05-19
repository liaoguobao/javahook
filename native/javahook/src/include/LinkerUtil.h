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
        if(!filename || !*filename)
            return 0;

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
            const char *symbol = 0;
            if(sdkver==24 || sdkver==25)//7.0\7.1
                symbol = "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv"; //void* do_dlopen(const char* filename, int flags, const android_dlextinfo* extinfo,       void* caller_addr)
            else//>=8.0
                symbol = "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv";//void* do_dlopen(const char* filename, int flags, const android_dlextinfo* extinfo, const void* caller_addr)

            *(void **)&do_dlopen = elfsym(sizeof(void*)==8 ? "linker64" : "linker", symbol);
        }
        if((void *)do_dlopen == (void *)-1)
        {
            //6.0及以下版本，对dlopen打开动态库没有任何限制，直接调用原始方法
            return ::dlopen(filename, flag);
        }
        if(do_dlopen)
        {
            size_t base; string path;
            CProcMaps::GetModulePathAndBase(filename, path, base);

            //注意第四个参数设置, 如果模块已经加载则设置为模块本身的基地址，如果模块未加载则设置为open函数的地址，从而绕过dlopen打开动态库的限制，
            //dlopen默认情况下不可以加载/system/lib目录下的尚未加载的动态库，对已加载的/system/lib下的动态库除了/etc/public.libraries.txt文件指定的外，其它动态库在调用dlsym函数时都是返回0
            return do_dlopen(filename, flag, 0, base?(void *)base:(void *)::open);
        }
        return 0;
    }
    static void* dlsym(const char *filename, const char *symbol, int isclose = 1)
    {
        if(!filename || !*filename)
            return 0;
        if(!symbol || !*symbol)
            return 0;

        void *h = CLinkerUtil::dlopen(filename, 0);
        void *s = h ? ::dlsym(h, symbol) : 0;
        if(isclose && h) ::dlclose(h);
        return s;
    }
    static void* elfsym(const char *filename, const char *symbol)
    {
        if(!filename || !*filename)
            return 0;
        if(!symbol || !*symbol)
            return 0;

        size_t base; string path;
        if(!CProcMaps::GetModulePathAndBase(filename, path, base))
            return 0;

        string elf;
        if(!CFileUtil::ReadBinDataFromFile(path.c_str(), elf))
            return 0;

        unsigned offset_sym = CElfUtil::GetSym_offset((unsigned char *)elf.c_str(), symbol);
        if(!offset_sym)
            return 0;

        void *sym = (void *)(base + offset_sym);
        return sym;
    }
};

#endif
