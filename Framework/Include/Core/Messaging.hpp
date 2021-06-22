//
#pragma once

#include "Logger.hpp"
#include <stdio.h>

// Logging functions.
#define R_LOG(chan, logType, format, ...) { \
    Recluse::Log r__log__(logType, chan); \
    Recluse::SizeT r__sz__ = snprintf(nullptr, 0, format, __VA_ARGS__); \
    r__log__.message.resize(r__sz__ + 1u); \
    snprintf((char*)r__log__.message.data(), r__log__.message.size(), format, __VA_ARGS__); \
}

#define R_ERR(chan, format, ...) R_LOG(chan, Recluse::LogError, format, __VA_ARGS__)
#define R_INFO(chan, format, ...) R_LOG(chan, Recluse::LogInfo, format, __VA_ARGS__)
#define R_WARN(chan, format, ...) R_LOG(chan, Recluse::LogWarn, format, __VA_ARGS__)
#define R_VERBOSE(chan, format, ...) R_LOG(chan, Recluse::LogVerbose, format, __VA_ARGS__)
#define R_TRACE(chan, format, ...) R_LOG(chan, Recluse::LogTrace, format, __VA_ARGS__)

#if !defined(RECLUSE_DEBUG)
#include <assert.h>
#define R_ASSERT_LOG()
#define R_ASSERT(expression) assert(expression)
#define R_DEBUG(chan, str, ...) R_LOG(chan, Recluse::LogDebug, str, __VA_ARGS__)
#else
#define R_ASSERT_LOG()
#define R_ASSERT()
#define R_DEBUG(str, ...)
#endif