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

        return REC_RESULT_INVALID_ARGS;
    }

    R_DEBUG(R_CHANNEL_WIN32, "Creating thread");
    //handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fun, 
    //            pThread->payload, 0, (LPDWORD)&pThread->uid);
    handle = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, startRoutine, pThread->payload, 0, (U32*)&pThread->uid));

    pThread->threadState    = THREAD_STATE_RUNNING;
    pThread->resultCode     = THREAD_RESULT_NOT_READY;
    
    exitCodeSuccess     = GetExitCodeThread(handle, &resultCode);
    pThread->resultCode = resultCode;

    if (handle == NULL) 
    {   
        R_ERR(R_CHANNEL_WIN32, "Failed to create thread! Result: %d", GetLastError());

        pThread->threadState = THREAD_STATE_UNKNOWN;

        return REC_RESULT_FAILED;
    }

    pThread->handle = handle;

    return REC_RESULT_OK;
}


ErrType joinThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);

    R_DEBUG(R_CHANNEL_WIN32, "Joining thread...");
    
    WaitForSingleObject(pThread->handle, INFINITE);
    GetExitCodeThread(pThread->handle, (LPDWORD)&pThread->resultCode);
    pThread->threadState = THREAD_STATE_NOT_RUNNING;

    return REC_RESULT_OK;
}


ErrType killThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);

    R_DEBUG(R_CHANNEL_WIN32, "Killing thread=%d ...", pThread->uid);

    TerminateThread(pThread->handle, 0);

    pThread->threadState = THREAD_STATE_UNKNOWN;

    return REC_RESULT_OK;
}


ErrType stopThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);
    R_ASSERT(pThread->threadState == THREAD_STATE_RUNNING); 

    SuspendThread(pThread->handle);
    
    pThread->threadState = THREAD_STATE_SUSPENDED;

    return REC_RESULT_OK;
}


ErrType resumeThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);
    R_ASSERT(pThread->threadState == THREAD_STATE_SUSPENDED);

    ResumeThread(pThread->handle);

    pThread->threadState = THREAD_STATE_RUNNING;    

    return REC_RESULT_OK;
}


Mutex createMutex(const char* name)
{
    name;
    HANDLE handle = CreateMutex(nullptr, FALSE, nullptr);
    return handle;
}


ErrType destroyMutex(Mutex mutex)
{
    if (!mutex) 
    {
        return REC_RESULT_NULL_PTR_EXCEPTION;
    }

    CloseHandle(mutex);
    
    return REC_RESULT_OK;
}


ErrType lockMutex(Mutex mutex)
{
    DWORD result = WaitForSingleObject(mutex, INFINITE);

    switch (result) 
    {
        case WAIT_OBJECT_0:
            return REC_RESULT_OK;

        default: 
            return REC_RESULT_FAILED;
    }

    return REC_RESULT_OK;
}


ErrType unlockMutex(Mutex mutex)
{
    ReleaseMutex(mutex);
    return REC_RESULT_OK;
}


ErrType waitMutex(Mutex mutex, U64 waitTimeMs)
{
    DWORD result = WaitForSingleObject(mutex, (DWORD)waitTimeMs);

    switch (result) 
    {
        case WAIT_OBJECT_0:
            return REC_RESULT_OK;

        case WAIT_TIMEOUT:
            return REC_RESULT_TIMEOUT;

        default:
            return REC_RESULT_FAILED;
    }
    
    return REC_RESULT_FAILED;
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