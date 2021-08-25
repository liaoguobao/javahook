// FileUtil.h

#ifndef _FILE_UTIL_H_
#define _FILE_UTIL_H_

#include "platform.h"

class CFileUtil
{
protected:
    CFileUtil()
    {
    }
    ~CFileUtil()
    {
    }

public:
    static BOOL IsDir(const char *dir)
    {
        if(!dir || !*dir)
            return FALSE;

        char buf[512] = {0};
        strcpy(buf, dir);
#if defined(_WIN32)
        int len = (int)strlen(buf);

        if(buf[len-1] == ':')
            buf[len] = '\\';
        if(len > 3 && (buf[len-1] == '\\' || buf[len-1] == '/'))
            buf[len-1] = 0;
#endif
        struct stat st;
        if(stat(buf, &st))
            return FALSE;

        BOOL b = ((st.st_mode & S_IFDIR)?TRUE:FALSE);
        return b;
    }

    static BOOL IsCanOpen(const char *file)
    {
        if(!file || !*file)
            return FALSE;

        int w = FALSE;
        int ok = TRUE;
        FILE *pf = NULL;

        do
        {
            if((pf = fopen(file, "r")))
                break;

            w = TRUE;
            if((pf = fopen(file, "w")))
                break;

            w = FALSE;
            ok = FALSE;
        }while(0);

        if(pf)
            fclose(pf);
        if(w)
            remove(file);

        return ok;
    }

    static BOOL IsExist(const char *file)
    {
        if(!file || !*file)
            return FALSE;

        int b = access(file, 0);
        return !b;
    }

    static BOOL CreateDir(const char *path)
    {
        if(!path || !*path)
            return FALSE;

        char szTmp[512];
        strcpy(szTmp, path);

        char szPath[512];
        szPath[0] = (path[0]==__DIRSEP__?__DIRSEP__:0);
        szPath[1] = 0;
#if defined(_WIN32)
#define __strtok__      strtok_s
#define __mkdir__(x, y) mkdir(x)
#else
#define __strtok__      strtok_r
#define __mkdir__(x, y) mkdir(x, y)
#endif
        int   szDelim = __DIRSEP__;
        char *pPos = 0;
        char *pNext = __strtok__(szTmp, (char *)&szDelim, &pPos);

        while(pNext)
        {
            strcat(szPath, pNext);
            strcat(szPath, (char *)&szDelim);

            pNext = __strtok__(0, (char *)&szDelim, &pPos);

            if(!access(szPath, 0))
                continue;

            if(__mkdir__(szPath, S_IRWXU))
                return FALSE;
        }

        return TRUE;
    }

    static BOOL GetLine(FILE *fp, string &s)
    {
        s.clear();
        if(!fp)
            return FALSE;

        unsigned len = 0;
        char buf[1024];

        while(fgets(buf, sizeof(buf), fp))
        {
            s.append(buf);
            len = (unsigned)s.size();

            if(!(s[len-1]==__DELIM__ || feof(fp)))
                continue;
#if defined(_WIN32)//遇到换行符或者到了文件结尾
            if(len >= 2 && *(unsigned short *)(s.c_str()+len-2) == '\r\n')
                len -= 2;
#endif
            if(len >= 1 && s[len-1] == __DELIM__)
                len -= 1;
            if(len != (unsigned)s.size())
                s.resize(len);
            return TRUE;
        }
        return FALSE;
    }

    static BOOL ReadBinDataFromFile(const char *file, string &data, int off = 0, int size = 0)
    {
        file = (IsDir(file) ? 0 : file);
        FILE *fp = (file ? fopen(file, "rb") : 0);
        BOOL b = ReadBinDataFromFp(fp, data, off, size);
        if(fp) fclose(fp);
        return b;
    }

    static BOOL WriteBinDataToFile(const char *file, const void *data, int size, int off = 0, int trunc = 1)
    {
        //wb  新建一个二进制文件，已存在的文件将内容清空，只允许写
        //r+b 打开一个二进制文件，文件必须存在，允许读写
        //trunc==1,off==-1 wb
        //trunc==1,off==0  wb
        //trunc==1,off> 0  wb
        //trunc==0,off==-1 r+b
        //trunc==0,off==0  r+b
        //trunc==0,off> 0  r+b
        off = (trunc==0 ? off : 0);//trunc模式下off字段无效
        const char *mode = (trunc==0 ? "r+b" : "wb");
        FILE *fp = (file ? fopen(file, mode) : 0);
        BOOL b = WriteBinDataToFp(fp, data, size, off);
        if(fp) fclose(fp);
        return b;
    }

    static BOOL ReadBinDataFromFp(FILE *fp, string &data, int off = 0, int size = 0)
    {
        int fd = (fp ? fileno(fp) : -1);
        BOOL b = ReadBinDataFromFd(fd, data, off, size);
        return b;
    }

    static BOOL WriteBinDataToFp(FILE *fp, const void *data, int size, int off = 0)
    {
        int fd = (fp ? fileno(fp) : -1);
        BOOL b = WriteBinDataToFd(fd, data, size, off);
        return b;
    }

    static BOOL ReadBinDataFromFd(int fd, string &data, int off = 0, int size = 0)
    {
        data.clear();
        if(fd<0 || off<0 || size<0)
            return FALSE;
        if(lseek(fd, off, SEEK_SET) == -1)
            return FALSE;

        char buf[0x1000];
        int once;
        do
        {
            once = (size==0 ? sizeof(buf) : size-(int)data.size());
            once = (once>(int)sizeof(buf) ? sizeof(buf) : once);

            if(once > 0)
                once = read(fd, buf, once);
            if(once > 0)
                data.append(buf, once);
            if(once == 0)
                return TRUE;
            if(once==-1 && errno!=EINTR)
                return FALSE;
        }while(1);
        return FALSE; //永远不会进到这里
    }

    static BOOL WriteBinDataToFd(int fd, const void *data, int size, int off = 0)
    {
        if(fd<0 || data==0 || size<=0 || off<-1)
            return FALSE;
        if(off!=-1 && lseek(fd, off, SEEK_SET)==-1)
            return FALSE;
        if(off==-1 && lseek(fd, 0, SEEK_END)==-1)
            return FALSE;

        int once = 0;
        do
        {
            if(size == 0)
                return TRUE;
            if(size > 0)
                once = write(fd, data, size);
            if(once > 0)
                data = (char *)data + once;
            if(once > 0)
                size = size - once;
            if(once==-1 && errno!=EINTR)
                return FALSE;
        }while(1);
        return FALSE; //永远不会进到这里
    }
};

#endif
