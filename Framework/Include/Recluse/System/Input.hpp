// 
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


// Poll events for the system.
R_PUBLIC_API void pollEvents();


enum GamePadInput
{
	GAME_PAD_INPUT_DPAD_UP,
	GAME_PAD_INPUT_DPAD_DOWN,
	GAME_PAD_INPUT_DPAD_LEFT,
	GAME_PAD_INPUT_DPAD_RIGHT,
	GAME_PAD_INPUT_L_THUMB,
	GAME_PAD_INPUT_R_THUMB,
	GAME_PAD_INPUT_START,
	GAME_PAD_INPUT_BACK,
	GAME_PAD_INPUT_L_SHOULDER,
	GAME_PAD_INPUT_R_SHOULDER,
	GAME_PAD_INPUT_X,
	GAME_PAD_INPUT_Y,
	GAME_PAD_INPUT_A,
	GAME_PAD_INPUT_B
};


struct GamePadIn
{
	U32 leftTrigger;
	U32 rightTrigger;
};
} // Recluse