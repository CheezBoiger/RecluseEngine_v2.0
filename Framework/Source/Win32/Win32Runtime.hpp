// 
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
        updateLastTimeS(getCurrentTickS());
    }

    U64 getLastTimeS() const;

    void updateLastTimeS(U64 newLastTimeS);

private:
    U64 m_time;
    
};

extern LRESULT CALLBACK win32RuntimeProc
                            (
                                HWND hwnd,
                                UINT uMsg, 
                                WPARAM wParam, 
                                LPARAM lParam
                            );



} // Recluse