//platform.h

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

//c99 header file
#include <assert.h>
//#include <inttypes.h>
#include <signal.h>
#include <stdlib.h>
//#include <complex.h>
//#include <iso646.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
//#include <stdbool.h>
//#include <tgmath.h>
#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <time.h>
//#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <wchar.h>
#include <float.h>
//#include <setjmp.h>
#include <stdio.h>
#include <wctype.h>

#ifdef  __cplusplus
//std header file
#include <string>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <stack>
//#include <deque>
//#include <set>
//#include <bitset>
using namespace std;

#endif

#if defined(_WIN32)
#define __DIRSEP__ '\\'
#else
#define __DIRSEP__ '/'
#endif

#if defined(__APPLE__)
#define __DELIM__  '\r'
#else
#define __DELIM__  '\n'
#endif

#if defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0601//win7
#endif

#ifndef WINVER
#define WINVER          0x0601//win7
#endif

#ifndef _WIN32_IE
#define _WIN32_IE       0x0800//IE8
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <conio.h>
#include <commctrl.h>
#include <richedit.h>
#include <winsock2.h>
#include <process.h>
#include <mstcpip.h>
#include <mswsock.h>
#include <oleacc.h>
#include <shellapi.h>
#include <iphlpapi.h>
#include <sys/stat.h>

#ifdef  __cplusplus
#include <atlbase.h>
#include <atlcomtime.h>
using namespace ATL;
#endif

#else//linux

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <netdb.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <dlfcn.h>
#include <termios.h>

#if defined(__APPLE__)
#include <sys/event.h>
#include <mach-o/fat.h>
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>
#include <execinfo.h>
#else
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <elf.h>
#endif

#if defined(__ANDROID__)
#include <jni.h>
#include <android/log.h>
#include <sys/system_properties.h>
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.

#endif

#endif

/*
Type      ILP32 LP64 LLP64 ILP64 LP32 
char      8     8    8     8     8    
short     16    16   16    16    16   
int       32    32   32    64    16   
long      32    64   32    64    32   
long long 64    64   64    64    64   
pointer   32    64   64    64    32   
float     32    32   32    32    32   
double    64    64   64    64    64   

LP32和ILP64数据模型目前极少被使用。
32位系统都采用的是ILP32数据模型。
64位Windows采用的是LLP64数据模型。
64位UNIX/Linux/MacOSX/ANDROID/IOS采用的是LP64数据模型。
*/
