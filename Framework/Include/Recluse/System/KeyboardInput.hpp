//
#pragma once

#include "Recluse/System/InputController.hpp"

namespace Recluse {

enum KeyStatus
{
	KEY_STATUS_UP,
	KEY_STATUS_DOWN,
	KEY_STATUS_STILL_DOWN
};

class Keyboard
{
public:
private:
	KeyStatus m_keys[256];
};
} // Recluse