//
#include "Core/Win32/Win32Common.hpp"
#include "Core/Win32/Win32Runtime.hpp"
#include "Core/RealtimeTick.hpp"

namespace Recluse {


struct {
    U64 gTicksPerSecond;
    U64 gTime;
} gWin32Runtime;


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