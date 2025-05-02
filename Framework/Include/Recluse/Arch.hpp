// Recluse 
#pragma once

#if defined(_DEBUG)
#ifndef RECLUSE_DEBUG
#define RECLUSE_DEBUG 1
#endif
#endif

#if defined(_WIN32)
    #include <vcruntime.h>
    #define WIN32_LEAN_AND_MEAN 1
    #define NOMINMAX 1
    #include <Windows.h>
    #define RECLUSE_WINDOWS 1
    // Called to allow exporting to public api. This will then expose to external modules.
    #define R_IMPORT_API __declspec(dllimport)
    #define R_EXPORT_API __declspec(dllexport)
    #define R_FORCE_INLINE __forceinline
    #define R_NOVTABLE __declspec(novtable)
    #define R_DEBUG_BREAK() do { __debugbreak(); } while(0)
    #define R_FORCE_CRASH(c) do { ExitProcess(c); } while(0)

    // Unsafe crash, this should only be used to force a segv, but there is a relatively uncommon
    // chance that this will produce undefined behavior, in which case, this isn't garaunteed to crash.
    #define R_FORCE_CRASH_UNSAFE() do { *(volatile unsigned long long *)0 = 0xDEADBEEF; } while (0)

    #define R_LIKELY(exp) __assume(exp)
    #define R_UNLIKELY(exp) __assume(!(exp))

    #define R_VECTORCALL __vectorcall
    #define R_FASTCALL __fastcall
    #define R_CDECL __cdecl
    #if defined(_M_X64) || defined(_M_AMD64)
        #define RECLUSE_64BIT
    #else
        #define RECLUSE_32BIT
    #endif
    #include <fenv.h>

    #if !defined(FE_INEXACT)
    #define FE_INEXACT   _SW_INEXACT       // _EM_INEXACT     0x00000001 inexact (precision)
    #endif
    #if !defined(FE_UNDERFLOW)
    #define FE_UNDERFLOW _SW_UNDERFLOW     // _EM_UNDERFLOW   0x00000002 underflow
    #endif
    #if !defined(FE_OVERFLOW)
    #define FE_OVERFLOW  _SW_OVERFLOW      // _EM_OVERFLOW    0x00000004 overflow
    #endif
    #if !defined(FE_DIVBYZERO)
    #define FE_DIVBYZERO _SW_ZERODIVIDE    // _EM_ZERODIVIDE  0x00000008 zero divide
    #endif
    #if !defined(FE_INVALID)
    #define FE_INVALID   _SW_INVALID       // _EM_INVALID     0x00000010 invalid
    #endif
#elif defined(__linux__)
    #error "Linux currently not supported for Recluse!"
    #define RECLUSE_LINUX
    #define R_EXPORT_API
    #define R_IMPORT_API
    #define R_FORCE_INLINE 
    #define R_NOVTABLE
    #define R_DEBUG_BREAK()
    #define R_FORCE_CRASH(c)
    #define R_FORCE_CRASH_UNSAFE()
    #define R_LIKELY(exp)
    #define R_UNLIKELY(exp)
    #define R_VECTORCALL
    #define R_FASTCALL 
    #define R_CDECL 
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

#define R_INTERNAL static
#define R_GLOBAL static

#if defined(R_DEVELOPER)
    #define RECLUSE_DEVELOPER 1
#endif