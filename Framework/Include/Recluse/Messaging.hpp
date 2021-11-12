//
#pragma once

#include "Recluse/Logger.hpp"
#include <stdio.h>

#if defined(_DEBUG)
#ifndef RECLUSE_DEBUG
#define RECLUSE_DEBUG 1
#endif
#endif

#if defined(R_DEVELOPER)
#define RECLUSE_DEVELOPER 1
#endif

// Logging functions.
#define R_LOG(chan, logType, format, ...) { \
    Recluse::Log r__log__(logType, chan); \
    Recluse::SizeT r__sz__ = snprintf(nullptr, 0, format, __VA_ARGS__); \
    r__log__.data.msg.resize(r__sz__ + 1u); \
    snprintf((char*)r__log__.data.msg.data(), r__log__.data.msg.size(), format, __VA_ARGS__); \
}

#define R_ERR(chan, format, ...) R_LOG(chan, Recluse::LogError, format, __VA_ARGS__)
#define R_INFO(chan, format, ...) R_LOG(chan, Recluse::LogInfo, format, __VA_ARGS__)
#define R_WARN(chan, format, ...) R_LOG(chan, Recluse::LogWarn, format, __VA_ARGS__)
#define R_VERBOSE(chan, format, ...) R_LOG(chan, Recluse::LogVerbose, format, __VA_ARGS__)
#define R_TRACE(chan, format, ...) R_LOG(chan, Recluse::LogTrace, format, __VA_ARGS__)

#if defined(RECLUSE_DEBUG) || defined(RECLUSE_DEVELOPER)
namespace Recluse {
namespace Asserts {

enum Result {
    ASSERT_OK,
    ASSERT_DEBUG,
    ASSERT_IGNORE,
    ASSERT_IGNORE_ALL,
    ASSERT_TERMINATE
};

// Assert handler deals with info needed to ensure, check, and debug code.
class AssertHandler {
public:
    static Result check(Bool cond, const char* functionStr, const char* msg);
};
} // Asserts
} // Recluse 
// Debugging macros and definitions. To be ignored on building release.
#include <assert.h>
#define R_ASSERT_LOG()
#define R_ASSERT(expression) assert(expression)
#define R_ASSERT_MSG(expression, msg) assert(expression && msg)
#define R_DEBUG(chan, str, ...) R_LOG(chan, Recluse::LogDebug, str, __VA_ARGS__)
#else
#define R_ASSERT_LOG()
#define R_ASSERT(expression)
#define R_ASSERT_MSG(expression, msg)
#define R_DEBUG(chan, str, ...)
#endif