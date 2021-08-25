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
            //6.0�����°汾����dlopen�򿪶�̬��û���κ����ƣ�ֱ�ӵ���ԭʼ����
            return ::dlopen(filename, flag);
        }
        if(do_dlopen)
        {
            //7.0�����ϰ汾��������ע����ĸ�����������open�����ĵ�ַ������linker,�˴ε����Ǵ�libc�з��𣬴Ӷ��ƹ�dlopen�򿪶�̬������ƣ�
            //dlopenĬ������²����Լ���/system/libĿ¼�µ���δ���صĶ�̬�⣬���Ѽ��ص�/system/lib�µĶ�̬�����/etc/public.libraries.txt�ļ�ָ�����⣬������̬���ڵ���dlsym����ʱ���Ƿ���0
            void *handle = do_dlopen(filename, flag, 0, (void *)::open);
            if(handle)//7.0\7.1\8.0\8.1����������ǰ����
                return handle;

            size_t base; string path;
            const char *n = strrchr(filename, '/');
            CProcMaps::GetModulePathAndBase(n?n+1:filename, path, base);

            //9.0�����ϰ汾��������ע����ĸ�����������ģ�鱾��Ļ���ַ������linker,�˴ε����Ƕ�̬�������𣬴Ӷ��ƹ�dlopen�򿪶�̬������ƣ�
            //dlopenĬ������²����Լ���/system/libĿ¼�µ���δ���صĶ�̬�⣬���Ѽ��ص�/system/lib�µĶ�̬�����/etc/public.libraries.txt�ļ�ָ�����⣬������̬���ڵ���dlsym����ʱ���Ƿ���0
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
