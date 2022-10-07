//
#include "Win32/IO/Win32Keyboard.hpp"

#include <unordered_map>

namespace Recluse {


static KeyStatus kWin32InputKeyCodes[KeyCode_MaxBufferCount];

#define R_WIN_KEY(dword, keycode) std::pair<DWORD, KeyCode>(dword, keycode)

// Key map from OS to engine agnostic.
// Maps are defined here: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
std::unordered_map<DWORD, KeyCode> kWin32KeyMap = 
{ 
    // TODO: Still need to register more.
    R_WIN_KEY(0x41, KeyCode_A),
    R_WIN_KEY(0x42, KeyCode_B),
    R_WIN_KEY(0x43, KeyCode_C),
    R_WIN_KEY(0x44, KeyCode_D),
    R_WIN_KEY(0x45, KeyCode_E),
    R_WIN_KEY(0x46, KeyCode_F),
    R_WIN_KEY(0x47, KeyCode_G),
    R_WIN_KEY(0x48, KeyCode_H),
    R_WIN_KEY(0x49, KeyCode_I),
    R_WIN_KEY(0x4A, KeyCode_J),
    R_WIN_KEY(0x4B, KeyCode_K),
    R_WIN_KEY(0x4C, KeyCode_L),
    R_WIN_KEY(0x4D, KeyCode_M),
    R_WIN_KEY(0x4E, KeyCode_N),
    R_WIN_KEY(0x4F, KeyCode_O),
    R_WIN_KEY(0x50, KeyCode_P),
    R_WIN_KEY(0x51, KeyCode_Q),
    R_WIN_KEY(0x52, KeyCode_R),
    R_WIN_KEY(0x53, KeyCode_S),
    R_WIN_KEY(0x54, KeyCode_T),
    R_WIN_KEY(0x55, KeyCode_U),
    R_WIN_KEY(0x56, KeyCode_V),
    R_WIN_KEY(0x57, KeyCode_W),
    R_WIN_KEY(0x58, KeyCode_X),
    R_WIN_KEY(0x59, KeyCode_Y),
    R_WIN_KEY(0x5A, KeyCode_Z),
    R_WIN_KEY(0x08, KeyCode_Backspace),
    R_WIN_KEY(0x09, KeyCode_Tab),
    R_WIN_KEY(0x10, KeyCode_Shift),
    R_WIN_KEY(0xA0, KeyCode_L_Shift),
    R_WIN_KEY(0xA1, KeyCode_R_Shift),
    R_WIN_KEY(0x20, KeyCode_Space),
    R_WIN_KEY(0x11, KeyCode_Control),
    R_WIN_KEY(0xA2, KeyCode_L_Control),
    R_WIN_KEY(0xA3, KeyCode_R_Control),
};


namespace Win32 {


KeyStatus getEngineStatus(DWORD status)
{
    switch ( status )
    {
        case WM_KEYDOWN:    return KeyStatus_Down;
        case WM_KEYUP:      return KeyStatus_Up;
        default:            return KeyStatus_Uknown;
    }
}

void registerKeyCall(DWORD keyCode, DWORD status)
{
    KeyStatus nowStatus     = getEngineStatus(status);
    KeyStatus currentStatus = kWin32InputKeyCodes[kWin32KeyMap[keyCode]];
    KeyStatus resolved      = nowStatus;

    if (currentStatus == KeyStatus_Down && nowStatus == KeyStatus_Down)
        resolved = KeyStatus_StillDown;

    kWin32InputKeyCodes[kWin32KeyMap[keyCode]] = resolved;
}
} // Win32


Bool isKeyUp(KeyCode code)
{
    return kWin32InputKeyCodes[code] == KeyStatus_Up;
}


Bool isKeyDown(KeyCode code)
{
    return kWin32InputKeyCodes[code] == KeyStatus_Down || kWin32InputKeyCodes[code] == KeyStatus_StillDown;
}


KeyStatus getKeyStatus(KeyCode code)
{
    return kWin32InputKeyCodes[code];
}
} // Recluse