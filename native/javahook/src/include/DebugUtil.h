//DebugUtil.h

#ifndef _DEBUG_UTIL_H_
#define _DEBUG_UTIL_H_

#include "platform.h"

class CDebugUtil
{
protected:
    CDebugUtil()
    {
    }
    ~CDebugUtil()
    {
    }

public:
    static const char * GetErrStr()
    {
        int err = 0;
#if !defined(_WIN32)
        err = errno;
#else
        err = ::GetLastError();
#endif
        return GetErrStrEx(err);
    }
    static const char * GetErrStrEx(int err)
    {
        static char errstr[1024] = {0};
#if !defined(_WIN32)
        strcpy(errstr, strerror(err));
#else
        ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, err, 0, errstr, sizeof(errstr), 0);
#endif
        return errstr;
    }
    static BOOL WritePlatformLog(const char *szFormat, ...)
    {
        string s;
        va_list args;

        va_start(args, szFormat);
        FormatToStringV(szFormat, args, s, 0);
        va_end(args);
#if defined(_WINDOWS)
        OutputDebugStringX("%s", s.c_str());
#elif defined(__ANDROID__)
        WriteToLogcat("%s", s.c_str());
#elif defined(__APPLE__)
        WriteToSyslog("%s", s.c_str());
#else//__LINUX__
        WriteToConsole("%s", s.c_str());
#endif
        return TRUE;
    }
    static BOOL GetModulePathAndBase(size_t addr, string &path, size_t &base)
    {
#if !defined(_WIN32)
        Dl_info dli = {0};
        BOOL hr = dladdr((void *)addr, &dli);

        path = dli.dli_fname;
        base = (size_t)dli.dli_fbase;
#else
        MEMORY_BASIC_INFORMATION mbi = {0};
        BOOL hr = (::VirtualQuery((void *)addr, &mbi, sizeof(MEMORY_BASIC_INFORMATION))?TRUE:FALSE);

        char buf[MAX_PATH] = {0};
        ::GetModuleFileNameA((HMODULE)mbi.AllocationBase, buf, sizeof(buf));

        path = buf;
        base = (size_t)mbi.AllocationBase;
#endif
        return hr;
    }
    static BOOL DeleteSelf()
    {
        string s;
        GetAbsolutePath(s);

        char cmd[256];
        const char *name = strrchr(s.c_str(), __DIRSEP__) + 1;
#if !defined(_WIN32)
        sprintf(cmd, "killall -9 %s && rm -f %s", name, s.c_str());
#else
        sprintf(cmd, "taskkill /F /IM %s && ping -n 2 localhost && del /F /Q %s", name, s.c_str());
#endif
        ExecShellCmd(cmd);
        return TRUE;
    }
    static BOOL RestartSelf(const char *update = 0, const char *cmdstr = 0, int mv = 1)
    {
        string s;
        GetAbsolutePath(s);

        if(!cmdstr)
            cmdstr = "";

        char cmd[256];
        const char *name = strrchr(s.c_str(), __DIRSEP__) + 1;
#if !defined(_WIN32)
        if(!update || !*update || !strcmp(update, s.c_str()))
            sprintf(cmd, "killall -9 %s && %s --shell %s", name, s.c_str(), cmdstr);
        else if(mv)
            sprintf(cmd, "killall -9 %s && mv -f %s %s && %s --shell %s", name, update, s.c_str(), s.c_str(), cmdstr);
        else
            sprintf(cmd, "killall -9 %s && cp -f %s %s && %s --shell %s", name, update, s.c_str(), s.c_str(), cmdstr);
#else
        if(!update || !*update || !strcmp(update, s.c_str()))
            sprintf(cmd, "taskkill /F /IM %s && ping -n 2 localhost && %s --shell %s", name, s.c_str(), cmdstr);
        else if(mv)
            sprintf(cmd, "taskkill /F /IM %s && ping -n 2 localhost && move /Y %s %s && %s --shell %s", name, update, s.c_str(), s.c_str(), cmdstr);
        else
            sprintf(cmd, "taskkill /F /IM %s && ping -n 2 localhost && copy /B /V /Y %s %s && %s --shell %s", name, update, s.c_str(), s.c_str(), cmdstr);
#endif
        ExecShellCmd(cmd);
        return TRUE;
    }
    static int CreateProc(const char * const argv[], int sigign = 1, int show = 0)
    {
        if(!argv || !argv[0] || !*argv[0])
            return FALSE;

        int pid = -1;
#if !defined(_WIN32)
        if((pid = fork()) == -1)
            return -1;
        if(pid && sigign)
            signal(SIGCHLD, SIG_IGN);
        if(!pid)
            execvp(argv[0], (char * const *)argv);
        if(!pid)
            _exit(127);
#else
        string param;
        for(int i = 1; argv && argv[i]; ++i)
        {
            const char *p = strchr(argv[i], ' ');

            param += p?"\"":"";
            param += argv[i];
            param += p?"\" ":" ";
        }

        SHELLEXECUTEINFOA ei = {sizeof(SHELLEXECUTEINFOA), SEE_MASK_NOCLOSEPROCESS, 0, "open", argv[0], param.c_str(), 0, show};
        pid = (::ShellExecuteExA(&ei)?::GetProcessId(ei.hProcess):-1);
        ::CloseHandle(ei.hProcess);
#endif
        return pid;
    }
    static BOOL KillProcByName(const char *name)
    {
        if(!name || !*name)
            return FALSE;

        char cmd[256];
#if !defined(_WIN32)
        sprintf(cmd, "killall -9 %s", name);
#else
        sprintf(cmd, "taskkill /F /IM %s", name);
#endif
        ExecShellCmd(cmd);
        return TRUE;
    }
#if !defined(_WIN32)
    static int GetPidByName(const char *name)
    {
        if(!name || !*name)
            return 0;

        char cmd[256];
#if defined(__APPLE__)
        sprintf(cmd, "killall -s %s", name);
#else
        sprintf(cmd, "pidof -s %s", name);
#endif
        string s;
        ExecShellCmdEx(cmd, s);
#if defined(__APPLE__)
        sscanf(s.c_str(), "%s%s%s", &cmd[64], &cmd[32], &cmd[0]);
        s = cmd;
#endif
        int pid = atoi(s.c_str());
        return pid;
    }

    static BOOL WriteToSyslog(const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        string out;
        va_list args;
        va_start(args, szFormat);
        FormatToStringV(szFormat, args, out, 0);
        va_end(args);

        string one;
        unsigned len = (unsigned)out.size();
        const char *log = out.c_str(), *end;

        for(unsigned i = 0; i < len; i += 900)
        {
            end = ((i + 900 < len) ? log + i + 900 : log + len);

            one = "WriteToSyslog: ";
            one.append(log + i, end);

            syslog(LOG_NOTICE, "%s", one.c_str());
        }

        return TRUE;
    }
#else
    static BOOL OutputDebugStringX(const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        string out;
        va_list args;
        va_start(args, szFormat);
        FormatToStringV(szFormat, args, out);
        va_end(args);

        ::OutputDebugStringA(out.c_str());
        return TRUE;
    }
    static BOOL OutputToRichEditWindow(HWND hRichEdit, UINT dwColor, const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;
        if(!::IsWindow(hRichEdit))
            return FALSE;
        if(!::SendMessageTimeoutA(hRichEdit, EM_SETSEL, -1, -1, SMTO_ABORTIFHUNG, 1000, NULL))
            return FALSE;

        CHARFORMATA cf = {0};
        cf.yHeight = 180;
        cf.cbSize = sizeof(CHARFORMATA);
        cf.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE;
        cf.crTextColor = dwColor;
        strcpy(cf.szFaceName, "新宋体");
        if(!::SendMessageTimeoutA(hRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, SMTO_ABORTIFHUNG, 1000, NULL))
            return FALSE;

        string out;
        va_list args;
        va_start(args, szFormat);
        FormatToStringV(szFormat, args, out);
        va_end(args);
        if(!::SendMessageTimeoutA(hRichEdit, EM_REPLACESEL, 0, (LPARAM)out.c_str(), SMTO_ABORTIFHUNG, 1000, NULL))
            return FALSE;

        return TRUE;
    }
    static int  MessageBoxX(HWND hWnd, const char *szCaption, UINT uType, const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        string out;
        va_list args;
        va_start(args, szFormat);
        FormatToStringV(szFormat, args, out);
        va_end(args);

        int ret = ::MessageBoxA(hWnd, out.c_str(), szCaption, uType);
        return ret;
    }
    static BOOL WriteToDebugView(const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        string out;
        va_list args;
        va_start(args, szFormat);
        FormatToStringV(szFormat, args, out, FALSE);
        va_end(args);

        string log = "\r\n";
        const char *sz = NULL;
        const char *szIn = out.c_str();

        while(*szIn)
        {
            sz = strchr(szIn, '\n');
            sz = (sz ? (sz + 1) : (out.c_str() + out.size()));

            log.append("WriteToDebugView: ");
            log.append(szIn, sz - szIn);

            szIn = sz;
        }

        ::OutputDebugStringA(log.c_str());
        return TRUE;
    }
#endif
    static BOOL WriteToConsole(const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        printf("[%s] ", TimevalToString(0));

        va_list args;
        va_start(args, szFormat);
        vprintf(szFormat, args);
        va_end(args);

        return TRUE;
    }

    static BOOL WriteToFile(const char *szFile, const char *szFormat, ...)
    {
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        FILE *pf = fopen(szFile, "a+b");
        if(pf == NULL)
            return FALSE;

        fseek(pf, 0, SEEK_END);

        fprintf(pf, "[%s] ", TimevalToString(0));

        va_list args;
        va_start(args, szFormat);
        vfprintf(pf, szFormat, args);
        va_end(args);

        fclose(pf);
        return TRUE;
    }

    static BOOL FormatToStringV(const char *szFormat, va_list &args, string &out, BOOL bTime = TRUE)
    {
        out.clear();
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        va_list argsT;
#if !defined(_WIN32)
        va_copy(argsT, args);
#else
        argsT = args;
#endif
        int iLen = vsnprintf(NULL, 0, szFormat, argsT) + 50;

        va_end(argsT);

        char *p = NULL;
        if((p = new char[iLen]) == NULL)
            return FALSE;

        int len1 = 0;

        if(bTime)
            len1 = sprintf(p, "[%s] ", TimevalToString(0));

        vsprintf(p + len1, szFormat, args);

        out = p;
        delete[] p;
        return TRUE;
    }

    static BOOL FormatToString(string &out, BOOL bTime, const char *szFormat, ...)
    {
        out.clear();
        if(szFormat == NULL || szFormat[0] == 0)
            return FALSE;

        va_list args;
        va_start(args, szFormat);
        BOOL b = FormatToStringV(szFormat, args, out, bTime);
        va_end(args);
        return b;
    }

    static void GetTimeVal(timeval &tv)
    {
#if !defined(_WIN32)
        gettimeofday(&tv, NULL);
#else
        FILETIME ft;
        ::GetSystemTimeAsFileTime(&ft);

        ULARGE_INTEGER __100ns__;
        __100ns__.LowPart = ft.dwLowDateTime;
        __100ns__.HighPart = ft.dwHighDateTime;
        __100ns__.QuadPart -= 116444736000000000;

        tv.tv_sec = (long)(__100ns__.QuadPart / 10000000);
        tv.tv_usec = (long)(__100ns__.QuadPart % 10000000) / 10;
#endif
    }

    static unsigned long long GetMilliseconds()
    {
        timeval tv;
        GetTimeVal(tv);

        return (unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000;
    }

    static void SleepMilliseconds(unsigned ms)
    {
#if !defined(_WIN32)
        unsigned sec = ms / 1000;
        unsigned usec = (ms % 1000) * 1000;

        if(sec > 0)
            sleep(sec);
        if(usec > 0)
            usleep(usec);
#else
        ::Sleep(ms);
#endif
    }

    static const char * TimevalToString(const timeval *ptv)
    {
        static char buf[32];
        buf[0] = 0;

        timeval tv;
        if(!ptv)
            GetTimeVal(tv);
        else
            tv = *ptv;

        time_t __time__ = tv.tv_sec;
        tm *ptm = localtime(&__time__);
        if(!ptm || (unsigned long)tv.tv_usec >= 1000000)
            return buf;
#if !defined(_WIN32)
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d.%03d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)tv.tv_usec/1000, (int)tv.tv_usec%1000);
#else
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)tv.tv_usec/1000);
#endif
        return buf;
    }

    static BOOL SetConsoleTextColor(int dwColor)
    {
#if defined(_WIN32)
        return ::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), dwColor);
#else
        dwColor &= ~FOREGROUND_INTENSITY;

        if(dwColor == FOREGROUND_GREEN)
            printf("\033[32m");//绿色
        else if(dwColor == FOREGROUND_RED)
            printf("\033[31m");//红色
        else if(dwColor == FOREGROUND_BLUE)
            printf("\033[34m");//蓝色
        else if(dwColor == (FOREGROUND_GREEN|FOREGROUND_RED))
            printf("\033[33m");//黄色
        else if(dwColor == (FOREGROUND_GREEN|FOREGROUND_BLUE))
            printf("\033[36m");//天蓝
        else if(dwColor == (FOREGROUND_BLUE|FOREGROUND_RED))
            printf("\033[35m");//紫色
        else if(dwColor == 0)
            printf("\033[30m");//黑色
        else if(dwColor == (FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN))
            printf("\033[37m");//白色
        else
            return FALSE;

        return TRUE;
#endif
    }

    static BOOL GetCurWorkDir(string &cwd)
    {
        cwd.clear();
        char buf[1024];
#if defined(_WIN32)
        if(!::GetCurrentDirectoryA(sizeof(buf), buf))
            return FALSE;
#else
        if(!getcwd(buf, sizeof(buf)))
            return FALSE;
#endif
        cwd = buf;
        return TRUE;
    }

    static BOOL SetCurWorkDir(const char *cwd)
    {
        if(!cwd || !*cwd)
            return FALSE;
#if defined(_WIN32)
        int b = ::SetCurrentDirectoryA(cwd);
#else
        int b = !chdir(cwd);
#endif
        return b;
    }

    static void * GetAppModuleHandle(const char *name)
    {
        string path;
        if(!name || !*name)
            GetAbsolutePath(path);
        else
            path = name;
#if defined(_WIN32)
        void *handle = (void *)::GetModuleHandleA(path.c_str());
#else
        void *handle = dlopen(path.c_str(), 0);
#endif
        return handle;
    }

    static BOOL GetAbsolutePath(string &path)
    {
        path.clear();
        char buf[256];
        int numb = sizeof(buf);
#if defined(__APPLE__)
        if(_NSGetExecutablePath(buf, (unsigned *)&numb) != 0)
            return FALSE;
        numb = strlen(buf);
#elif defined(__linux__)
        if((numb = readlink("/proc/self/exe", buf, numb)) <= 0)
            return FALSE;
#else
        if((numb = ::GetModuleFileNameA(NULL, buf, numb)) <= 0)
            return FALSE;
#endif
        path.assign(buf, numb);
        return TRUE;
    }
#ifdef __ANDROID__
    static BOOL WriteToLogcat(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        BOOL b = 0;
        if(fmt == (char *)(-1))
        {
            void *__android_log_loggable = (void *)va_arg(args, size_t);
            b = AndroidLogBufWrite(0, ANDROID_LOG_INFO, (char *)__android_log_loggable, (char *)(-1));
        }
        else
        {
            string log; FormatToStringV(fmt, args, log, 0);
            b = AndroidLogBufWrite(0, ANDROID_LOG_INFO, "WriteToLogcat", "%s", log.c_str());
        }
        va_end(args);
        return b;
    }
    static BOOL AndroidLogBufWrite(int bufid, int prio, const char *tag, const char *fmt, ...)
    {
        static void*(*__android_log_loggable) () = 0; //在loghook.cpp的HOOK_onload函数中被设置为loghook_android_log_loggable
        static int  (*__android_log_buf_write)(int bufid, int prio, const char *tag, const char *msg) = 0;

        if(fmt == (char *)(-1))
        {
            *(void **)&__android_log_loggable = (void *)tag;
            return TRUE;
        }
        void* addr = 0;
        if(__android_log_loggable)
        {
            addr = __android_log_loggable();
        }
        if(!__android_log_buf_write && addr)
        {
            *(void **)&__android_log_buf_write = addr;
        }
        string log;
        va_list args;
        va_start(args, fmt);
        FormatToStringV(fmt, args, log, 0);
        va_end(args);

        size_t len = log.size();
        char *msg = (char *)log.c_str(), *end, c;

        char tag_[64];
        sprintf(tag_, "%s@%d", tag, gettid());

        for(size_t i = 0; i < len; i += 1023)
        {
            end = ((i + 1023 <= len) ? msg + 1023 : (char *)log.c_str() + len);
            c = *end;
            *end = 0;
            if(!__android_log_buf_write)
            {
                __android_log_print(prio, tag_, "%s", msg); //>1023个字符时会被截断
            }
            else
            {
                __android_log_buf_write(bufid, prio, tag_, msg);
            }
            *end = c;
            msg = end;
        }
        return TRUE;
    }
    static BOOL GetAppProcessName(string &name)
    {
        name.clear();

        string exe;
        if(!GetAbsolutePath(exe))
            return FALSE;

        if( exe != "/system/bin/app_process" &&
            exe != "/system/bin/app_process32" &&
            exe != "/system/bin/app_process64" &&
            exe != "/system/bin/app_process_xposed" &&
            exe != "/system/bin/app_process32_xposed" &&
            exe != "/system/bin/app_process64_xposed")
            return FALSE;

        int fd = open("/proc/self/cmdline", O_RDONLY);
        if(fd <= 0)
            return FALSE;

        char buf[1024];
        int len = read(fd, buf, sizeof(buf));
        close(fd);
        if(len <= 0)
            return FALSE;
        if(!strcmp(buf, "<pre-initialized>"))
            return FALSE;

        name = buf;
        return TRUE;
    }
#endif
    static BOOL ExecShellCmd(const char *cmdstr, int wait = 1, int show = 0)
    {
        if(!cmdstr || !*cmdstr)
            return FALSE;

        int pid;
        const char *argv[4];
#if !defined(_WIN32)
        argv[0] = "sh";
        argv[1] = "-c";
#else
        argv[0] = "cmd";
        argv[1] = "/C";
#endif
        argv[2] = cmdstr;
        argv[3] = 0;

        if((pid = CreateProc(argv, !wait, show)) <= 0)
            return FALSE;
        if(!wait)
            return TRUE;
#if !defined(_WIN32)
        waitpid(pid, 0, 0);
#else
        HANDLE h = ::OpenProcess(SYNCHRONIZE, FALSE, pid);
        ::WaitForSingleObject(h, INFINITE);
        ::CloseHandle(h);
#endif
        return TRUE;
    }

    static BOOL ExecShellCmdEx(const char *cmdstr, string &out)
    {
        out.clear();

        if(!cmdstr || !*cmdstr)
            return FALSE;

        string _cmdstr = cmdstr;
        _cmdstr += " 2>&1";

#if !defined(_WIN32)
        FILE *pf = popen(_cmdstr.c_str(), "r");
#else
        FILE *pf = _popen(_cmdstr.c_str(), "r");
#endif
        if(!pf)
            return FALSE;

        char _buf[0x1000];
        unsigned _one;

        do
        {
            _one = (unsigned)fread(_buf, 1, sizeof(_buf), pf);
            out.append(_buf, _one);
        }while(_one == sizeof(_buf));

#if !defined(_WIN32)
        pclose(pf);
#else
        _pclose(pf);
#endif
        return TRUE;
    }
    static BOOL IntToStr(const void *p, unsigned len, string &out, int align = 4, BOOL upper = TRUE)
    {
        out.clear();
        if(!p || !len)
            return FALSE;
        if(align!=1 && align!=2 && align!=4 && align!=8)
            return FALSE;

        char buf[32];
        unsigned num = len/align;

        if(align == 1)
        {
            const unsigned char *ele = (unsigned char *)p;
            const char *fmt = (upper?", [%d]0x%02X":", [%d]0x%02x");

            for(unsigned i = 0; i < num; ++i)
            {
                sprintf(buf, fmt, i, ele[i]);
                out += buf;
            }
        }
        else if(align == 2)
        {
            const unsigned short *ele = (unsigned short *)p;
            const char *fmt = (upper?", [%d]0x%04X":", [%d]0x%04x");

            for(unsigned i = 0; i < num; ++i)
            {
                sprintf(buf, fmt, i, ele[i]);
                out += buf;
            }
        }
        else if(align == 4)
        {
            const unsigned int *ele = (unsigned int *)p;
            const char *fmt = (upper?", [%d]0x%08X":", [%d]0x%08x");

            for(unsigned i = 0; i < num; ++i)
            {
                sprintf(buf, fmt, i, ele[i]);
                out += buf;
            }
        }
        else if(align == 8)
        {
            const unsigned long long *ele = (unsigned long long *)p;
            const char *fmt = (upper?", [%d]0x%016llX":", [%d]0x%016llx");

            for(unsigned i = 0; i < num; ++i)
            {
                sprintf(buf, fmt, i, ele[i]);
                out += buf;
            }
        }
        if(!out.empty())
        {
            out = out.substr(2);
        }

        sprintf(buf, "[%d]int{", num);
        out = buf + out + "}";
        return TRUE;
    }
};

#endif
