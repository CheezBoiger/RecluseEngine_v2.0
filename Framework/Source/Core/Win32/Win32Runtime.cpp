//
#include "Core/Win32/Win32Common.hpp"
#include "Core/Win32/Win32Runtime.hpp"
#include "Core/RealtimeTick.hpp"

#include "Core/Messaging.hpp"
#include "Core/Logging/LogFramework.hpp"

namespace Recluse {


struct {
    U64 gTicksPerSecond;
    U64 gTime;
} gWin32Runtime;


void enableOSColorInput()
{
    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (stdOutHandle == INVALID_HANDLE_VALUE) {
            
        R_ERR(R_CHANNEL_WIN32, "Unable to obtain standard output handle.");

        return;

    }

    DWORD dwMode = 0; 

    if (!GetConsoleMode(stdOutHandle, &dwMode)) {

        R_ERR(R_CHANNEL_WIN32, "Unable to get output handle mode!");

        return;

    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;   
    
    if (!SetConsoleMode(stdOutHandle, dwMode)) {
    
        R_ERR(R_CHANNEL_WIN32, "Unable to set the output handle mode for virtual terminal processing!");
    
    }
}


static void initializePerformanceTimer()
{
    LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);

    gWin32Runtime.gTicksPerSecond   = freq.QuadPart;
    gWin32Runtime.gTime             = counter.QuadPart;
}


void updateLastTimeS(U64 newLastTimeS)
{
    gWin32Runtime.gTime = newLastTimeS;
}


U64 getTicksPerSecondS()
{
    return gWin32Runtime.gTicksPerSecond;
}


U64 getLastTimeS()
{
    return gWin32Runtime.gTime;
}

U64 getCurrentTickS()
{
    LARGE_INTEGER newTick;
    QueryPerformanceCounter(&newTick);
    return newTick.QuadPart;
}

RealtimeTick::RealtimeTick()
{
    U64 ticksPerSecond  = getTicksPerSecondS();
    U64 lastTimeS       = getLastTimeS();
    U64 currentTimeS    = getCurrentTickS();

    m_deltaTimeS        = F32(currentTimeS - lastTimeS) / ticksPerSecond;
    m_currentTimeS      = F32(currentTimeS);
    gWin32Runtime.gTime = currentTimeS;
}


void RealtimeTick::initialize()
{
    initializePerformanceTimer();
}


RealtimeTick RealtimeTick::GetTick()
{
    return RealtimeTick();
}
} // Recluse