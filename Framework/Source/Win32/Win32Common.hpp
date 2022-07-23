// 
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"

#ifndef RECLUSE_WINDOWS
#error "Windows environment variable was not defined! Maybe other environment is defined?"
#endif 

#include <Windows.h>
#include <Xinput.h>

#define R_CHANNEL_WIN32 "Win32"

// We will comment the xinput library to be utilized for controller support.
#pragma comment(lib, "XInput.lib")