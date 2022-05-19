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
            //6.0�����°汾����dlopen�򿪶�̬��û���κ����ƣ�ֱ�ӵ���ԭʼ����
            return ::dlopen(filename, flag);
        }
        if(do_dlopen)
        {
            size_t base; string path;
            CProcMaps::GetModulePathAndBase(filename, path, base);

            //ע����ĸ���������, ���ģ���Ѿ�����������Ϊģ�鱾��Ļ���ַ�����ģ��δ����������Ϊopen�����ĵ�ַ���Ӷ��ƹ�dlopen�򿪶�̬������ƣ�
            //dlopenĬ������²����Լ���/system/libĿ¼�µ���δ���صĶ�̬�⣬���Ѽ��ص�/system/lib�µĶ�̬�����/etc/public.libraries.txt�ļ�ָ�����⣬������̬���ڵ���dlsym����ʱ���Ƿ���0
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
