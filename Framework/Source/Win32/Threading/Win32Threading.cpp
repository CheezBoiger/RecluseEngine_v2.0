// 
#include "Win32/Threading/Win32Thread.hpp"
#include "Recluse/Messaging.hpp"

#include <process.h>

namespace Recluse {


ResultCode createThread(Thread* pThread, ThreadFunction startRoutine)
{
    SetConsoleMode(nullptr, ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    DWORD resultCode = 0;
    HANDLE handle = NULL;
    BOOL exitCodeSuccess = FALSE;
    
    if (!pThread) 
    {
        R_ERROR(R_CHANNEL_WIN32, "pThread input passed is null! This is not valid!");

        return RecluseResult_InvalidArgs;
    }

    R_DEBUG(R_CHANNEL_WIN32, "Creating thread");
    //handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fun, 
    //            pThread->payload, 0, (LPDWORD)&pThread->uid);
    handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)startRoutine, pThread->payload, 0, (LPDWORD)&pThread->uid);

    pThread->threadState    = ThreadState_Running;
    pThread->resultCode     = ThreadResultCode_NotReady;
    
    exitCodeSuccess     = GetExitCodeThread(handle, &resultCode);
    pThread->resultCode = resultCode;

    if (handle == NULL) 
    {   
        R_ERROR(R_CHANNEL_WIN32, "Failed to create thread! Result: %d", GetLastError());

        pThread->threadState = ThreadState_Unknown;

        return RecluseResult_Failed;
    }

    pThread->handle = handle;

    return RecluseResult_Ok;
}


ResultCode joinThread(Thread* pThread)

{
    R_STATIC_ASSERT(pThread != NULL);

    R_DEBUG(R_CHANNEL_WIN32, "Joining thread...");
    
    WaitForSingleObject(pThread->handle, INFINITE);
    GetExitCodeThread(pThread->handle, (LPDWORD)&pThread->resultCode);
    pThread->threadState = ThreadState_NotRunning;

    return RecluseResult_Ok;
}


ResultCode killThread(Thread* pThread)
{
    R_STATIC_ASSERT(pThread != NULL);

    R_DEBUG(R_CHANNEL_WIN32, "Killing thread=%d ...", pThread->uid);

    TerminateThread(pThread->handle, 0);

    pThread->threadState = ThreadState_Unknown;

    return RecluseResult_Ok;
}


ResultCode stopThread(Thread* pThread)
{
    R_STATIC_ASSERT(pThread != NULL);
    R_STATIC_ASSERT(pThread->threadState == ThreadState_Running); 

    SuspendThread(pThread->handle);
    
    pThread->threadState = ThreadState_Suspended;

    return RecluseResult_Ok;
}


ResultCode resumeThread(Thread* pThread)
{
    R_STATIC_ASSERT(pThread != NULL);
    R_STATIC_ASSERT(pThread->threadState == ThreadState_Suspended);

    ResumeThread(pThread->handle);

    pThread->threadState = ThreadState_Running;    

    return RecluseResult_Ok;
}


Mutex createMutex(const char* name)
{
    name;
    HANDLE handle = CreateMutex(nullptr, FALSE, name);
    return handle;
}


ResultCode destroyMutex(Mutex mutex)
{
    if (!mutex) 
    {
        return RecluseResult_NullPtrExcept;
    }

    CloseHandle(mutex);
    
    return RecluseResult_Ok;
}


ResultCode lockMutex(Mutex mutex, U64 waitMs)
{
    DWORD waitTimeMs = (waitMs == kInfiniteMs) ? INFINITE : waitMs;
    DWORD result = WaitForSingleObject(mutex, waitTimeMs);

    switch (result) 
    {
        case WAIT_OBJECT_0:
            return RecluseResult_Ok;

        case WAIT_ABANDONED:

        case WAIT_TIMEOUT:

        default: 
            return RecluseResult_Failed;
    }

    return RecluseResult_Failed;
}


ResultCode unlockMutex(Mutex mutex)
{
    ReleaseMutex(mutex);
    return RecluseResult_Ok;
}


ResultCode waitMutex(Mutex mutex, U64 waitTimeMs)
{
    DWORD result = WaitForSingleObject(mutex, (DWORD)waitTimeMs);

    switch (result) 
    {
        case WAIT_OBJECT_0:
            return RecluseResult_Ok;

        case WAIT_TIMEOUT:
            return RecluseResult_Timeout;

        default:
            return RecluseResult_Failed;
    }
    
    return RecluseResult_Failed;
}

ResultCode tryLockMutex(Mutex mutex)
{
    // Wait 10 milliseconds, perhaps shorter?
    ResultCode result = lockMutex(mutex, 0ull);
    return result;
}

Semaphore createSemaphore(const char* name)
{
    HANDLE semaphoreHandle = CreateSemaphore(NULL, 0, 16, name);
    return semaphoreHandle;
}


U64 compareExchange(I64* dest, I64 ex, I64 comp)
{
    return InterlockedCompareExchange64((LONG64*)dest, ex, comp);
}


I16 compareExchange(I16* dest, I16 ex, I16 comp)
{
    return InterlockedCompareExchange16((SHORT*)dest, ex, comp);
}

U128 compareExchange(U128* dest, U128 ex, U128 comp)
{
    R_NO_IMPL();
    return U128();
}


u32 testAndSet(volatile uptr ptr, U32 offset)
{
    return InterlockedBitTestAndSet((LONG*)ptr, (LONG)offset);
}


ResultCode CriticalSection::initialize()
{
    R_STATIC_ASSERT_FORMAT(m_section == NULL, "Critical Section is not null prior to initialization! Could indicate was already created? section=%d", m_section);
    m_section = malloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSection((LPCRITICAL_SECTION)m_section);
    return RecluseResult_Ok;
}


ResultCode CriticalSection::release()
{
    R_STATIC_ASSERT(m_section != NULL);
    DeleteCriticalSection((LPCRITICAL_SECTION)m_section);
    ::free(m_section);
    m_section = NULL;
    return RecluseResult_Ok;
}


ResultCode CriticalSection::enter()
{
    R_STATIC_ASSERT(m_section != NULL);
    EnterCriticalSection((LPCRITICAL_SECTION)m_section);
    return RecluseResult_Ok;
}


ResultCode CriticalSection::tryEnter()
{
    R_STATIC_ASSERT(m_section != NULL);
    BOOL success = TryEnterCriticalSection((LPCRITICAL_SECTION)m_section);
    return (success ? RecluseResult_Ok : RecluseResult_Failed);
}


ResultCode CriticalSection::leave()
{
    R_STATIC_ASSERT(m_section != NULL);
    LeaveCriticalSection((LPCRITICAL_SECTION)m_section);
    return RecluseResult_Ok;
}


i32 fetchAdd(uptr ptr, i32 arg)
{
    LONG result = InterlockedAdd((LONG*)ptr, (LONG)arg);
    return (i32)result;
}

i32 fetchSub(uptr ptr, i32 arg)
{
    LONG result = InterlockedAdd((LONG*)ptr, (LONG)-arg);
    return (i32)result;
}
} // Recluse