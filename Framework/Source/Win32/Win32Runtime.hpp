// 
#pragma once

#include "Win32/Win32Common.hpp"

namespace Recluse {


// Grab ticks per second.
U64 getTicksPerSecondS();
U64 getCurrentTickS();

class Win32RuntimeTick 
{
public:
    Win32RuntimeTick()
    {
        updateLastTimeS(getCurrentTickS(), 0.f);
    }

    U64 getLastTimeS() const;
    F32 getCurrentTime() const { return m_currentTimeS; }
    F32 getDelta() const { return m_deltaTimeS; }

    void updateLastTimeS(U64 newLastTimeS, F32 deltaTime);

    static void overrideRuntimeTick(U32 threadId, U32 clockId, F32 newDelta);

private:
    U64 m_time;
    F32 m_currentTimeS;
    F32 m_deltaTimeS;
    
};

extern LRESULT CALLBACK win32RuntimeProc
                            (
                                HWND hwnd,
                                UINT uMsg, 
                                WPARAM wParam, 
                                LPARAM lParam
                            );



} // Recluse