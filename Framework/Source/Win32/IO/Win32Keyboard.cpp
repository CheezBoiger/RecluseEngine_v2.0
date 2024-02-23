//
#include "Win32/IO/Win32Keyboard.hpp"

#include <unordered_map>

namespace Recluse {

static KeyStatus kWin32InputKeyCodes[KeyCode_MaxBufferCount];

#define R_WIN_KEY(dword, keycode) std::pair<DWORD, KeyCode>(dword, keycode)
#define R_KEY_WIN(dword, keycode) std::pair<KeyCode, DWORD>(keycode, dword)

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
    R_WIN_KEY(0x14, KeyCode_Capslock),
    R_WIN_KEY(0x12, KeyCode_Alt),
    R_WIN_KEY(0xA4, KeyCode_L_Alt),
    R_WIN_KEY(0xA5, KeyCode_R_Alt),
    R_WIN_KEY(0x30, KeyCode_0),
    R_WIN_KEY(0x31, KeyCode_1),
    R_WIN_KEY(0x32, KeyCode_2),
    R_WIN_KEY(0x33, KeyCode_3),
    R_WIN_KEY(0x34, KeyCode_4),
    R_WIN_KEY(0x35, KeyCode_5),
    R_WIN_KEY(0x36, KeyCode_6),
    R_WIN_KEY(0x37, KeyCode_7),
    R_WIN_KEY(0x38, KeyCode_8),
    R_WIN_KEY(0x39, KeyCode_9),
    R_WIN_KEY(0x70, KeyCode_F1),
    R_WIN_KEY(0x71, KeyCode_F2),
    R_WIN_KEY(0x72, KeyCode_F3),
    R_WIN_KEY(0x73, KeyCode_F4),
    R_WIN_KEY(0x74, KeyCode_F5),
    R_WIN_KEY(0x75, KeyCode_F6),
    R_WIN_KEY(0x76, KeyCode_F7),
    R_WIN_KEY(0x77, KeyCode_F8),
    R_WIN_KEY(0x78, KeyCode_F9),
    R_WIN_KEY(0x79, KeyCode_F10),
    R_WIN_KEY(0x7A, KeyCode_F11),
    R_WIN_KEY(0x7B, KeyCode_F12),
    R_WIN_KEY(0x1B, KeyCode_Escape),
};


std::unordered_map<KeyCode, DWORD> kWin32KeyCodeToDWORDMap = 
{
    // TODO: Still need to register more.
    R_KEY_WIN(0x41, KeyCode_A),
    R_KEY_WIN(0x42, KeyCode_B),
    R_KEY_WIN(0x43, KeyCode_C),
    R_KEY_WIN(0x44, KeyCode_D),
    R_KEY_WIN(0x45, KeyCode_E),
    R_KEY_WIN(0x46, KeyCode_F),
    R_KEY_WIN(0x47, KeyCode_G),
    R_KEY_WIN(0x48, KeyCode_H),
    R_KEY_WIN(0x49, KeyCode_I),
    R_KEY_WIN(0x4A, KeyCode_J),
    R_KEY_WIN(0x4B, KeyCode_K),
    R_KEY_WIN(0x4C, KeyCode_L),
    R_KEY_WIN(0x4D, KeyCode_M),
    R_KEY_WIN(0x4E, KeyCode_N),
    R_KEY_WIN(0x4F, KeyCode_O),
    R_KEY_WIN(0x50, KeyCode_P),
    R_KEY_WIN(0x51, KeyCode_Q),
    R_KEY_WIN(0x52, KeyCode_R),
    R_KEY_WIN(0x53, KeyCode_S),
    R_KEY_WIN(0x54, KeyCode_T),
    R_KEY_WIN(0x55, KeyCode_U),
    R_KEY_WIN(0x56, KeyCode_V),
    R_KEY_WIN(0x57, KeyCode_W),
    R_KEY_WIN(0x58, KeyCode_X),
    R_KEY_WIN(0x59, KeyCode_Y),
    R_KEY_WIN(0x5A, KeyCode_Z),
    R_KEY_WIN(0x08, KeyCode_Backspace),
    R_KEY_WIN(0x09, KeyCode_Tab),
    R_KEY_WIN(0x10, KeyCode_Shift),
    R_KEY_WIN(0xA0, KeyCode_L_Shift),
    R_KEY_WIN(0xA1, KeyCode_R_Shift),
    R_KEY_WIN(0x20, KeyCode_Space),
    R_KEY_WIN(0x11, KeyCode_Control),
    R_KEY_WIN(0xA2, KeyCode_L_Control),
    R_KEY_WIN(0xA3, KeyCode_R_Control),
    R_KEY_WIN(0x14, KeyCode_Capslock),
    R_KEY_WIN(0x12, KeyCode_Alt),
    R_KEY_WIN(0xA4, KeyCode_L_Alt),
    R_KEY_WIN(0xA5, KeyCode_R_Alt),
    R_KEY_WIN(0x30, KeyCode_0),
    R_KEY_WIN(0x31, KeyCode_1),
    R_KEY_WIN(0x32, KeyCode_2),
    R_KEY_WIN(0x33, KeyCode_3),
    R_KEY_WIN(0x34, KeyCode_4),
    R_KEY_WIN(0x35, KeyCode_5),
    R_KEY_WIN(0x36, KeyCode_6),
    R_KEY_WIN(0x37, KeyCode_7),
    R_KEY_WIN(0x38, KeyCode_8),
    R_KEY_WIN(0x39, KeyCode_9),
    R_KEY_WIN(0x70, KeyCode_F1),
    R_KEY_WIN(0x71, KeyCode_F2),
    R_KEY_WIN(0x72, KeyCode_F3),
    R_KEY_WIN(0x73, KeyCode_F4),
    R_KEY_WIN(0x74, KeyCode_F5),
    R_KEY_WIN(0x75, KeyCode_F6),
    R_KEY_WIN(0x76, KeyCode_F7),
    R_KEY_WIN(0x77, KeyCode_F8),
    R_KEY_WIN(0x78, KeyCode_F9),
    R_KEY_WIN(0x79, KeyCode_F10),
    R_KEY_WIN(0x7A, KeyCode_F11),
    R_KEY_WIN(0x7B, KeyCode_F12),
    R_KEY_WIN(0x1B, KeyCode_Escape),
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
    else if (currentStatus == KeyStatus_StillDown && nowStatus == KeyStatus_Down)
        resolved = KeyStatus_StillDown;

    kWin32InputKeyCodes[kWin32KeyMap[keyCode]] = resolved;
}


void monitorKeyDown(KeyCode keyCode)
{
    if (kWin32InputKeyCodes[keyCode] == KeyStatus_Down)
    {
        SHORT status = GetAsyncKeyState(kWin32KeyCodeToDWORDMap[keyCode]);
        if (((0x8000) & status))
        {
            // key is still down.
            kWin32InputKeyCodes[keyCode] = KeyStatus_StillDown;
        }
    }
}
} // Win32


Bool KeyboardListener::isKeyUp(KeyCode code)
{
    Win32::monitorKeyDown(code);
    return kWin32InputKeyCodes[code] == KeyStatus_Up;
}


Bool KeyboardListener::isKeyDown(KeyCode code)
{
    Win32::monitorKeyDown(code);
    return kWin32InputKeyCodes[code] == KeyStatus_Down || kWin32InputKeyCodes[code] == KeyStatus_StillDown;
}


KeyStatus KeyboardListener::getKeyStatus(KeyCode code)
{
    Win32::monitorKeyDown(code);
    return kWin32InputKeyCodes[code];
}


Bool KeyboardListener::isKeyStillDown(KeyCode keyCode)
{
    Win32::monitorKeyDown(keyCode);
    return kWin32InputKeyCodes[keyCode];
}


Bool KeyboardListener::isKeyDownOnce(KeyCode keyCode)
{
    Bool isDownOnce = kWin32InputKeyCodes[keyCode] == KeyStatus_Down;
    Win32::monitorKeyDown(keyCode);
    return isDownOnce;
}
} // Recluse