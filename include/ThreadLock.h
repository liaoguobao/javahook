//ThreadLock.h

#ifndef _THREADLOCK_H_
#define _THREADLOCK_H_

#include "platform.h"

class CThreadLock
{
protected:
#if !defined(_WIN32)
    pthread_mutex_t     m_tm;
    pthread_mutexattr_t m_attr;
#else
    CRITICAL_SECTION    m_cs;
#endif

public:
    CThreadLock()
    {
#if !defined(_WIN32)
        pthread_mutexattr_init(&m_attr);
        pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_tm, &m_attr);
#else
        ::InitializeCriticalSection(&m_cs);
#endif
    }
    virtual ~CThreadLock()
    {
#if !defined(_WIN32)
        pthread_mutex_destroy(&m_tm);
        pthread_mutexattr_destroy(&m_attr);
#else
        ::DeleteCriticalSection(&m_cs);
#endif
    }
    void Lock()
    {
#if !defined(_WIN32)
        while(pthread_mutex_lock(&m_tm));
#else
        ::EnterCriticalSection(&m_cs);
#endif
    }
    void UnLock()
    {
#if !defined(_WIN32)
        while(pthread_mutex_unlock(&m_tm));
#else
        ::LeaveCriticalSection(&m_cs);
#endif
    }
};

class CThreadLockHelper
{
protected:
    unsigned        m_dwCount;
    CThreadLock   * m_pTL;

public:
    CThreadLockHelper(CThreadLock *pTL, BOOL bLock = TRUE)
    {
        m_dwCount = 0;
        m_pTL = pTL;
        if(bLock)
            Lock();
    }
    virtual ~CThreadLockHelper()
    {
        while(m_dwCount)
            UnLock();
    }
    void Lock()
    {
        ++m_dwCount;
        m_pTL->Lock();
    }
    void UnLock()
    {
        if(m_dwCount > 0)
        {
            --m_dwCount;
            m_pTL->UnLock();
        }
    }
};

#endif
