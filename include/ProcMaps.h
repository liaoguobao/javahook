//ProcMaps.h

#ifndef _PROC_MAPS_H_
#define _PROC_MAPS_H_

#include "platform.h"

typedef BOOL (*PROCMAPSCALLBACK)(const char *, const char *, const char *, const char *, const char *, const char *, void *);

class CProcMaps
{
protected:
    CProcMaps()
    {
    }
    ~CProcMaps()
    {
    }

public:
    static BOOL IsModuleLoaded(const char *name)
    {
        string path;
        size_t base;
        BOOL b = GetModulePathAndBase(name, path, base);
        return b;
    }
    static BOOL GetModulePathAndBase(const char *name, string &path, size_t &base)
    {
        base = 0;
        path.clear();

        if(!name || !*name)
            return FALSE;

        struct __data__ { size_t base; string path; } data = { 0, name };
        if(!TraversalProcMaps(GetModulePathAndBase_callback, &data))
            return FALSE;

        path = data.path;
        base = data.base;
        return TRUE;
    }
    static BOOL GetModulePathAndBase(size_t addr, string &path, size_t &base)
    {
        base = 0;
        path.clear();

        if(!addr)
            return FALSE;

        /*struct __data__ { size_t base; string path; } data = { addr, "" };
        if(!TraversalProcMaps(GetModulePathAndBase_callback, &data))
            return FALSE;

        path = data.path;
        base = data.base;*/

        Dl_info dli = {0};
        if(!dladdr((void *)addr, &dli))
            return FALSE;

        path = dli.dli_fname;
        base = (size_t)dli.dli_fbase;
        return TRUE;
    }
    static BOOL TraversalProcMaps(PROCMAPSCALLBACK callback, void *data)
    {
        if(!callback)
            return FALSE;

        char sf[32];
        sprintf(sf, "/proc/%d/maps", getpid());

        FILE *pf = fopen(sf, "rb");
        if(!pf)
            return FALSE;

        char field[1024];
        char address[20], perms[8], offset[16], dev[8], inode[8], name[256];
        int ret = FALSE, len;

        while(TRUE)
        {
            if(!fgets(field, sizeof(field), pf))
                break;

            len = (int)strlen(field);

            if((len >= 1) && (field[len-1] == '\r' || field[len-1] == '\n'))
                field[len-1] = 0;
            if((len >= 2) && (field[len-2] == '\r' || field[len-2] == '\n'))
                field[len-2] = 0;
            if(!field[0])
                continue;

            address[0] = perms[0] = offset[0] = dev[0] = inode[0] = name[0] = 0;

            if(sscanf(field, "%s%s%s%s%s%s", address, perms, offset, dev, inode, name) != 6)
                continue;
            if(!callback(address, perms, offset, dev, inode, name, data))
                continue;

            ret = TRUE;
            break;
        }

        fclose(pf);
        return ret;
    }

protected:
    static BOOL GetModulePathAndBase_callback(const char *address, const char *perms, const char *offset, const char *dev, const char *inode, const char *name, void *data)
    {
        size_t addr_b, addr_e, addr = *(size_t *)data;
        const char *_name, *so = ((string *)((size_t *)data + 1))->c_str();

        //if(perms[2] != 'x')
        //    return FALSE;
        if(sscanf(address, "%x-%x", &addr_b, &addr_e) != 2)
            return FALSE;

        if(*so)
        {
            _name = strrchr(name, '/');
            _name = (_name ? _name+1 : name);

            if(strcmp(_name, so))
                return FALSE;
        }
        else
        {
            if(addr < addr_b || addr >= addr_e)
                return FALSE;
        }

        *(string *)((size_t *)data + 1) = name;
        *(size_t *)data = addr_b;
        return TRUE;
    }
};

#endif
