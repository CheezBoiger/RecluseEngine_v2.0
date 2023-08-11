//
#pragma once

#include "Recluse/Logger.hpp"
#include "Recluse/Arch.hpp"
#include "Recluse/System/DateTime.hpp"
#include <stdio.h>

// Logging functions.
#define R_LOG(chan, logType, format, ...) \
    { \
        Recluse::Log r__log__(logType, chan); \
        Recluse::SizeT r__sz__ = snprintf(nullptr, 0, format, __VA_ARGS__); \
        r__log__.data.msg.resize(r__sz__ + 1u); \
        snprintf((char*)r__log__.data.msg.data(), r__log__.data.msg.size(), format, __VA_ARGS__); \
        r__log__ << Recluse::DateFormatter("%Y-%M-%D %h:%m:%s"); \
    }

// Helper macros for logging messages.
#define R_INFO(chan, format, ...)       do { R_LOG(chan, Recluse::LogInfo, format, __VA_ARGS__);    } while (false)
#define R_WARN(chan, format, ...)       do { R_LOG(chan, Recluse::LogWarn, format, __VA_ARGS__);    } while (false)
#define R_VERBOSE(chan, format, ...)    do { R_LOG(chan, Recluse::LogVerbose, format, __VA_ARGS__); } while (false)
#define R_TRACE(chan, format, ...)      do { R_LOG(chan, Recluse::LogTrace, format, __VA_ARGS__);   } while (false)
#define R_NOTIFY(chan, format, ...)     do { R_LOG(chan, Recluse::LogNotify, format, __VA_ARGS__);  } while (false)
#define R_DEBUG(chan, str, ...)         do { R_LOG(chan, Recluse::LogDebug, str, __VA_ARGS__); } while (false)
 
#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)
    namespace Recluse {
    namespace Asserts {

    enum Result 
    {
        ASSERT_OK,
        ASSERT_DEBUG,
        ASSERT_IGNORE,
        ASSERT_IGNORE_ALL,
        ASSERT_TERMINATE
    };

    // Assert handler deals with info needed to ensure, check, and debug code.
    class AssertHandler 
    {
    public:
        static Result check(Bool cond, const char* functionStr, const char* msg);
    };
    } // Asserts
    } // Recluse 

    // Debugging macros and definitions. To be ignored on building release.
    #include <assert.h>
    #define R_ASSERT_LOG()
    #if !defined(R_IGNORE_ASSERT)
        #if defined(RECLUSE_WINDOWS)
            #define R_ASSERT(expression) do { _set_error_mode(_OUT_TO_MSGBOX); assert(expression); } while (0)
            #define R_ASSERT_FORMAT(expression, fmt, ...) \
                do { \
                    wchar_t werr[512]; { \
                    char err[512]; \
                    sprintf(err, #expression ## ", " ## #fmt, __VA_ARGS__); \
                    MultiByteToWideChar(CP_UTF8, 0, err, 512, werr, 512); \
                    } \
                    _set_error_mode(_OUT_TO_MSGBOX); \
                    (void)((!!(expression)) || (_wassert(werr, _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0)); \
                } while (0)
        #else
            // TODO: For anything other than windows, we still need to improve this.
            #define R_ASSERT(expression) assert(expression)
            #define R_ASSERT_FORMAT(expression, msg, ...) do { char err[512]; sprintf(err, #msg, __VA_ARGS__); assert(expression && err); } while (0)
        #endif
    #else
        #undef R_DEBUG_BREAK()
        #define R_DEBUG_BREAK()
        #define R_ASSERT(expression)
        #define R_ASSERT_FORMAT(expression, msg, ...)
    #endif // !defined(R_IGNORE_ASSERT)
    #define R_DEBUG_WRAP(cond) cond
#else
    #define R_ASSERT_LOG()
    #define R_ASSERT(expression)
    #define R_ASSERT_FORMAT(expression, msg, ...)
    #define R_DEBUG_WRAP(cond)
#endif

#if defined(RECLUSE_DEVELOPER)
    #if defined(RECLUSE_DEBUG)
        #define R_ERROR(chan, format, ...) \
            do { \
                R_LOG(chan, Recluse::LogError, format, __VA_ARGS__); \
                R_DEBUG_BREAK(); \
            } while (false)

        // Call an interrupt to instruct a fatal error.
        #define R_FATAL_ERROR(chan, format, ...) \
            do { \
                R_LOG(chan, Recluse::LogFatal, format, __VA_ARGS__); \
                R_DEBUG_BREAK(); \
                R_FORCE_CRASH(-1); \
            } while (false)
    #else
        #define R_ERROR(chan, format, ...) do { R_LOG(chan, Recluse::LogFatal, format, __VA_ARGS__); } while (false)
        #define R_FATAL_ERROR(chan, format, ...) do { R_LOG(chan, Recluse::LogFatal, format, __VA_ARGS__); } while (false)
    #endif
    // NOTE(): We should always be implementing a function when needed, but for development purposes,
    // we can simply place a warning assert to let us know it is not written. Otherwise, to 
    // the user, this is entirely ignored.
    #define R_NO_IMPL() R_ASSERT_FORMAT(false, "No implementation for %s", __FUNCTION__)
#else
    #define R_ERROR(chan, format, ...) do { R_LOG(chan, Recluse::LogError, format, __VA_ARGS__); } while (false)
    #define R_FATAL_ERROR(chan, format, ...) do { R_LOG(chan, Recluse::LogFatal, format, __VA_ARGS__); } while (false)
    #define R_NO_IMPL()
#endif