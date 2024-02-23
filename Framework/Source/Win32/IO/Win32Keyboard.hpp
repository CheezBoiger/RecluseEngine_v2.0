//

#pragma once

#include "Recluse/System/KeyboardInput.hpp"
#include "Win32/Win32Common.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace Win32 {

void registerKeyCall(DWORD keycode, DWORD status);
} // Win32
} // Recluse