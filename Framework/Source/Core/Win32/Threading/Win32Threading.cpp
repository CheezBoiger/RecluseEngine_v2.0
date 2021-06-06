// 
#include "Core/Win32/Threading/Win32Thread.hpp"
#include "Core/Messaging.hpp"

namespace Recluse {


ErrType createThread(Thread* pThread, ThreadFunction startRoutine)
{
    DWORD resultCode = 0;
    HANDLE handle = NULL;
    BOOL exitCodeSuccess = FALSE;
    if (!pThread) {

        R_ERR(R_CHANNEL_WIN32, "pThread input passed is null! This is not valid!");

        return -1;
    }

    R_DEBUG(R_CHANNEL_WIN32, "Creating thread");

    handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startRoutine, 
                pThread->payload, 0, (LPDWORD)&pThread->uid);
    
    exitCodeSuccess = GetExitCodeThread(handle, &resultCode);
    pThread->resultCode = resultCode;

    if (handle == NULL) {
       
        R_ERR(R_CHANNEL_WIN32, "Failed to create thread! Result: %d", GetLastError());
        
        return -2;
    }

    return 0;
}
} // Recluse