// 
#pragma once

#include "Core/Arch.hpp"
#include "Core/Types.hpp"

#ifndef RECLUSE_WINDOWS
#error "Windows environment variable was not defined! Maybe other environment is defined?"
#endif 

#include <Windows.h>

#define R_CHANNEL_WIN32 "Win32"