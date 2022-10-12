// Recluse 
#pragma once

#if defined(_DEBUG)
#ifndef RECLUSE_DEBUG
#define RECLUSE_DEBUG 1
#endif
#endif

#if defined(_WIN32)
    #include <vcruntime.h>
    #define RECLUSE_WINDOWS 1
    // Called to allow exporting to public api. This will then expose to external modules.
    #define R_PUBLIC_API __declspec(dllexport)
    #define R_IMPORT __declspec(dllimport)
    #define R_FORCEINLINE __forceinline
    #define R_NOVTABLE __declspec(novtable)
    #define R_DEBUG_BREAK() do { __debugbreak(); } while(0)
    #define R_FORCE_CRASH(c) do { ExitProcess(c); } while(0)
    #if defined(_M_X64) || defined(_M_AMD64)
        #define RECLUSE_64BIT
    #else
        #define RECLUSE_32BIT
    #endif
#elif defined(__linux__)
    #error "Linux currently not supported for Recluse!"
    #define RECLUSE_LINUX
    #define R_PUBLIC_API
    #define R_IMPORT
    #define R_FORCEINLINE 
    #define R_NOVTABLE
    #define R_DEBUG_BREAK()
    #define R_FORCE_CRASH(c)
#else
    #error "Architecture not supported for Recluse!"
#endif 

#if !defined(__cplusplus)
    #if !defined(NULL)
        #define NULL ((void*)0)
    #endif
#else
    #if !defined(NULL)
        #define NULL 0
    #endif
#endif

// Tags that help define what is Operating system specific, that must be implemented.
#define R_OS_SPECIFIC
// Tag used to indicate what is Operating system specific, that must be implemented.
#define R_OS_CALL     R_OS_SPECIFIC