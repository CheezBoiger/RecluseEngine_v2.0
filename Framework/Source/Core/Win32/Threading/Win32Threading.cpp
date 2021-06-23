// 
#include "Core/Win32/Threading/Win32Thread.hpp"
#include "Core/Messaging.hpp"

namespace Recluse {


ErrType createThread(Thread* pThread, ThreadFunction startRoutine)
{
    SetConsoleMode(nullptr, ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    DWORD resultCode = 0;
    HANDLE handle = NULL;
    BOOL exitCodeSuccess = FALSE;
    if (!pThread) {

        R_ERR(R_CHANNEL_WIN32, "pThread input passed is null! This is not valid!");

        return REC_RESULT_INVALID_ARGS;
    }

    R_DEBUG(R_CHANNEL_WIN32, "Creating thread");

    handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startRoutine, 
                pThread->payload, 0, (LPDWORD)&pThread->uid);
    
    exitCodeSuccess     = GetExitCodeThread(handle, &resultCode);
    pThread->resultCode = resultCode;

    if (handle == NULL) {
       
        R_ERR(R_CHANNEL_WIN32, "Failed to create thread! Result: %d", GetLastError());
        
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

    return REC_RESULT_OK;
}


ErrType killThread(Thread* pThread)
{
    R_ASSERT(pThread != NULL);

    TerminateThread(pThread->handle, 0);
    
    return REC_RESULT_OK;
}


void* createMutex()
{
    HANDLE handle = CreateMutex(nullptr, FALSE, nullptr);
    return handle;
}


ErrType destroyMutex(void* mutex)
{
    if (!mutex) {

        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    CloseHandle(mutex);
    
    return REC_RESULT_OK;
}


ErrType lockMutex(void* mutex)
{
    DWORD result = WaitForSingleObject(mutex, INFINITE);

    switch (result) {

        case WAIT_OBJECT_0:
            return REC_RESULT_OK;

        default: 
            return REC_RESULT_FAILED;

    }

    return REC_RESULT_OK;
}


ErrType unlockMutex(void* mutex)
{
    ReleaseMutex(mutex);
    return REC_RESULT_OK;
}


ErrType waitMutex(void* mutex, U64 waitTimeMs)
{
    DWORD result = WaitForSingleObject(mutex, (DWORD)waitTimeMs);

    switch (result) {

        case WAIT_OBJECT_0:
            return REC_RESULT_OK;

        case WAIT_TIMEOUT:
            return REC_RESULT_TIMEOUT;

        default:
            return REC_RESULT_FAILED;

    }
    
    return REC_RESULT_FAILED;
}
} // Recluse