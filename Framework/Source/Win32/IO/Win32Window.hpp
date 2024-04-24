//
#pragma once

#include "Win32/Win32Common.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/Threading/Threading.hpp"

#define R_WIN32_WINDOW_NAME L"RecluseWindow"
#define R_WIN32_PROP_NAME L"RecluseProp"

namespace Recluse {

extern CriticalSection windowCs;

// Exposed for d3d12 only.
R_PUBLIC_API Window* getWindowAssociatedWithHwnd(HWND hwnd);
} // Recluse