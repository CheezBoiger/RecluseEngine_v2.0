//
#pragma once

#include "Recluse/System/InputController.hpp"
#include "Recluse/Arch.hpp"

namespace Recluse {

enum KeyStatus
{
	KeyStatus_Up,
	KeyStatus_Down,
	KeyStatus_StillDown,
	KeyStatus_Uknown
};

// All keys from a QWERTY key board, defined as such.
enum KeyCode
{
	KeyCode_Q,
	KeyCode_W,
	KeyCode_E,
	KeyCode_R,
	KeyCode_T,
	KeyCode_Y,
	KeyCode_U,
	KeyCode_I,
	KeyCode_O,
	KeyCode_P,
	KeyCode_A,
	KeyCode_S,
	KeyCode_D,
	KeyCode_F,
	KeyCode_G,
	KeyCode_H,
	KeyCode_J,
	KeyCode_K,
	KeyCode_L,
	KeyCode_Z,
	KeyCode_X,
	KeyCode_C,
	KeyCode_V,
	KeyCode_B,
	KeyCode_N,
	KeyCode_M,
	KeyCode_Up,
	KeyCode_Down,
	KeyCode_Left,
	KeyCode_Right,
	KeyCode_Semicolon,
	KeyCode_L_Paranthesis,
	KeyCode_R_Paranthesis,
	KeyCode_L_Bracket,
	KeyCode_R_Bracket,
	KeyCode_L_Square_Bracket,
	KeyCode_R_Square_Bracket,
	KeyCode_Colon,
	KeyCode_Quote,
	KeyCode_Double_Quote,
	KeyCode_Back_Slash,
	KeyCode_Slash,
	KeyCode_Plus,
	KeyCode_Minus,
	KeyCode_Underscore,
	KeyCode_Equals,
	KeyCode_1,
	KeyCode_2,
	KeyCode_3,
	KeyCode_4,
	KeyCode_5,
	KeyCode_6,
	KeyCode_7,
	KeyCode_8,
	KeyCode_9,
	KeyCode_0,
	KeyCode_Period,
	KeyCode_Comma,
	KeyCode_Exclamation,
	KeyCode_At,
	KeyCode_Pound,
	KeyCode_Dash,
	KeyCode_Dollar,
	KeyCode_Percent,
	KeyCode_Caret,
	KeyCode_Ampersand,
	KeyCode_Asterisk,
	KeyCode_Backspace,
	KeyCode_Space,
	KeyCode_Control, // Any control button.
	KeyCode_L_Control,
	KeyCode_R_Control,
	KeyCode_Shift,	// Any shift button.
	KeyCode_L_Shift,
	KeyCode_R_Shift,
	KeyCode_Alt,	// Any alt button.
	KeyCode_L_Alt,
	KeyCode_R_Alt,
	KeyCode_Pipe,
	KeyCode_Bar = KeyCode_Pipe,
	KeyCode_VBar = KeyCode_Pipe,
	KeyCode_Tilde,
	KeyCode_Backtick,
	KeyCode_Tab,
	KeyCode_Capslock,
	KeyCode_F1,
	KeyCode_F2,
	KeyCode_F3,
	KeyCode_F4,
	KeyCode_F5,
	KeyCode_F6,
	KeyCode_F7,
	KeyCode_F8,
	KeyCode_F9,
	KeyCode_F10,
	KeyCode_F11,
	KeyCode_F12,
	KeyCode_Printscreen,
	KeyCode_Scroll_Lock,
	KeyCode_Pause,
	KeyCode_Page_Up,
	KeyCode_Page_Down,
	KeyCode_Insert,
	KeyCode_Delete,
	KeyCode_Home,
	KeyCode_End,
	KeyCode_Escape,
	KeyCode_L_Special,	// Special left button, mainly an OS specific reserved key.
	KeyCode_R_Special,	// Special right button, mainly an OS specific reserved key.
	KeyCode_MaxBufferCount
};


class R_PUBLIC_API KeyboardListener
{
public:
	KeyboardListener() { }

	// Check if the key is up and idle.
	Bool		isKeyUp(KeyCode code);
	// Check if the key is pressed down.
	Bool		isKeyDown(KeyCode code);
	// Checks if the key is still down.
	Bool		isKeyStillDown(KeyCode code);
	// Check if the key is pressed down once. Should the key be checked if held down,
	// You probaly want to use isKeyDown(), or isKeyStillDown()
	Bool		isKeyDownOnce(KeyCode code);
	// Get the key status.
	KeyStatus	getKeyStatus(KeyCode code);
};
} // Recluse