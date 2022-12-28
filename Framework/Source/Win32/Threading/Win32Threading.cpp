// 
#include "Win32/Threading/Win32Thread.hpp"
#include "Recluse/Messaging.hpp"

#include <process.h>

namespace Recluse {


ErrType createThread(Thread* pThread, ThreadFunction startRoutine)
{
    SetConsoleMode(nullptr, ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    DWORD resultCode = 0;
    HANDLE handle = NULL;
    BOOL exitCodeSuccess = FALSE;
    
    if (!pThread) 
    {
        R_ERR(R_CHANNEL_WIN32, "pThread input passed is null! This is not valid!");

        return RecluseResult_InvalidArgs;
    }

    R_DEBUG(R_CHANNEL_WIN32, "Creating thread");
    //handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fun, 
    //            pThread->payload, 0, (LPDWORD)&pThread->uid);
    handle = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, startRoutine, pThread->payload, 0, (U32*)&pThread->uid));

    pThread->threadState    = ThreadState_Running;
    pThread->resultCode     = ThreadResultCode_NotReady;
    
    exitCodeSuccess     = GetExitCodeThread(handle, &resultCode);
    pThread->resultCode = resultCode;

    if (handle == NULL) 
    {   
        R_ERR(R_CHANNEL_WIN32, "Failed to create thread! Result: %d", GetLastError());

        pThread->threadState = ThreadState_Unknown;

        return RecluseResult_Failed;
    }

    pThread->handle = handle;

    return RecluseResult_Ok;
}


ErrType joinThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);

    R_DEBUG(R_CHANNEL_WIN32, "Joining thread...");
    
    WaitForSingleObject(pThread->handle, INFINITE);
    GetExitCodeThread(pThread->handle, (LPDWORD)&pThread->resultCode);
    pThread->threadState = ThreadState_NotRunning;

    return RecluseResult_Ok;
}


ErrType killThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);

    R_DEBUG(R_CHANNEL_WIN32, "Killing thread=%d ...", pThread->uid);

    TerminateThread(pThread->handle, 0);

    pThread->threadState = ThreadState_Unknown;

    return RecluseResult_Ok;
}


ErrType stopThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);
    R_ASSERT(pThread->threadState == ThreadState_Running); 

    SuspendThread(pThread->handle);
    
    pThread->threadState = ThreadState_Suspended;

    return RecluseResult_Ok;
}


ErrType resumeThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);
    R_ASSERT(pThread->threadState == ThreadState_Suspended);

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


ErrType destroyMutex(Mutex mutex)
{
    if (!mutex) 
    {
        return RecluseResult_NullPtrExcept;
    }

    CloseHandle(mutex);
    
    return RecluseResult_Ok;
}


ErrType lockMutex(Mutex mutex)
{
    DWORD result = WaitForSingleObject(mutex, INFINITE);

    switch (result) 
    {
        case WAIT_OBJECT_0:
            return RecluseResult_Ok;

        default: 
            return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}


ErrType unlockMutex(Mutex mutex)
{
    ReleaseMutex(mutex);
    return RecluseResult_Ok;
}


ErrType waitMutex(Mutex mutex, U64 waitTimeMs)
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

ErrType tryLockMutex(Mutex mutex)
{
    // Wait 10 milliseconds, perhaps shorter?
    ErrType result = waitMutex(mutex, 12ull);
    return result;
}

Semaphore createSemaphore()
{
    HANDLE semaphoreHandle = CreateSemaphore(NULL, 0, 16, "RecluseSemaphore");
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
} // Recluse