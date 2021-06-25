// 
#include "Core/Win32/Win32Common.hpp"

namespace Recluse {

// Grab ticks per second.
U64 getTicksPerSecondS();
U64 getLastTimeS();
U64 getCurrentTickS();

void updateLastTimeS(U64 newLastTimeS);

extern LRESULT CALLBACK win32RuntimeProc(HWND hwnd,
  UINT uMsg, WPARAM wParam, LPARAM lParam);

} // Recluse