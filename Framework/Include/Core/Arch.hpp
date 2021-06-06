// Recluse 
#pragma once

#include "Core/Types.hpp"

#if defined(_WIN32)
#define RECLUSE_WINDOWS
#define R_EXPORT __declspec(dllexport)
#define R_IMPORT __declspec(dllimport)
#if defined(_M_X64) || defined(_M_AMD64)
    #define RECLUSE_64BIT
#else
    #define RECLUSE_32BIT
#endif
#elif deifned(__linux__)
#define RECLUSE_LINUX
#define R_EXPORT
#define R_IMPORT
#else
    #error "Architecture not supported for Recluse!"
#endif 
