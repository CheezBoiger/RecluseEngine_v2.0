//
#pragma once

#include "Recluse/System/InputController.hpp"

namespace Recluse {

enum KeyStatus
{
	KEY_STATUS_UNKNOWN,
	KEY_STATUS_STILL_UP,
	KEY_STATUS_UP,
	KEY_STATUS_DOWN,
	KEY_STATUS_STILL_DOWN
};

// All keys from a QWERTY key board, defined as such.
enum KeyCode
{
	KEY_CODE_Q,
	KEY_CODE_W,
	KEY_CODE_E,
	KEY_CODE_R,
	KEY_CODE_T,
	KEY_CODE_Y,
	KEY_CODE_U,
	KEY_CODE_I,
	KEY_CODE_O,
	KEY_CODE_P,
	KEY_CODE_A,
	KEY_CODE_S,
	KEY_CODE_D,
	KEY_CODE_F,
	KEY_CODE_G,
	KEY_CODE_H,
	KEY_CODE_J,
	KEY_CODE_K,
	KEY_CODE_L,
	KEY_CODE_Z,
	KEY_CODE_X,
	KEY_CODE_C,
	KEY_CODE_V,
	KEY_CODE_B,
	KEY_CODE_N,
	KEY_CODE_M,
	KEY_CODE_UP,
	KEY_CODE_DOWN,
	KEY_CODE_LEFT,
	KEY_CODE_RIGHT,
	KEY_CODE_SEMICOLON,
	KEY_CODE_L_PARANTHESIS,
	KEY_CODE_R_PARANTHESIS,
	KEY_CODE_L_BRACKET,
	KEY_CODE_R_BRACKET,
	KEY_CODE_L_SQUARE_BRACKET,
	KEY_CODE_R_SQUARE_BRACKET,
	KEY_CODE_COLON,
	KEY_CODE_QUOTE,
	KEY_CODE_DOUBLE_QUOTE,
	KEY_CODE_BACK_SLASH,
	KEY_CODE_SLASH,
	KEY_CODE_PLUS,
	KEY_CODE_MINUS,
	KEY_CODE_UNDERSCORE,
	KEY_CODE_EQUALS,
	KEY_CODE_1,
	KEY_CODE_2,
	KEY_CODE_3,
	KEY_CODE_4,
	KEY_CODE_5,
	KEY_CODE_6,
	KEY_CODE_7,
	KEY_CODE_8,
	KEY_CODE_9,
	KEY_CODE_0,
	KEY_CODE_PERIOD,
	KEY_CODE_COMMA,
	KEY_CODE_EXCLAMATION,
	KEY_CODE_AT,
	KEY_CODE_POUND,
	KEY_CODE_DOLLAR,
	KEY_CODE_PERCENT,
	KEY_CODE_CARET,
	KEY_CODE_AMPERSAND,
	KEY_CODE_ASTERISK,
	KEY_CODE_BACKSPACE,
	KEY_CODE_SPACE,
	KEY_CODE_L_CONTROL,
	KEY_CODE_R_CONTROL,
	KEY_CODE_L_SHIFT,
	KEY_CODE_R_SHIFT,
	KEY_CODE_L_ALT,
	KEY_CODE_R_ALT,
	KEY_CODE_PIPE,
	KEY_CODE_BAR = KEY_CODE_PIPE,
	KEY_CODE_VBAR = KEY_CODE_PIPE,
	KEY_CODE_TILDE,
	KEY_CODE_BACKTICK,
	KEY_CODE_TAB,
	KEY_CODE_CAPSLOCK,
	KEY_CODE_F1,
	KEY_CODE_F2,
	KEY_CODE_F3,
	KEY_CODE_F4,
	KEY_CODE_F5,
	KEY_CODE_F6,
	KEY_CODE_F7,
	KEY_CODE_F8,
	KEY_CODE_F9,
	KEY_CODE_F10,
	KEY_CODE_F11,
	KEY_CODE_F12,
	KEY_CODE_PRINTSCREEN,
	KEY_CODE_SCROLL_LOCK,
	KEY_CODE_PAUSE,
	KEY_CODE_PAGE_UP,
	KEY_CODE_PAGE_DOWN,
	KEY_CODE_INSERT,
	KEY_CODE_DELETE,
	KEY_CODE_HOME,
	KEY_CODE_END,
	KEY_CODE_ESCAPE
};

Bool isKeyUp(KeyCode code);
Bool isKeyDown(KeyCode code);
KeyStatus getKeyStatus(KeyCode code);
} // Recluse