//
#pragma once

#include "Recluse/Types.hpp"

#include <chrono>
#include <varargs.h>

namespace Recluse {


enum LogType {
    LogMsg          = 0,
    LogError        = (1 << 0),
    LogDebug        = (1 << 1),
    LogWarn         = (1 << 2),
    LogVerbose      = (1 << 3),
    LogInfo         = (1 << 4),
    LogDontStore    = (1 << 5),
    LogTrace        = (1 << 6)
};

typedef U32 LogTypeFlags;

struct LogMessage {
    std::string msg;
    std::string channel;
    LogType type;
};

struct Log {
    LogMessage data;

    static R_EXPORT void initializeLoggingSystem();
    static R_EXPORT void destroyLoggingSystem();

    Log(LogType type = LogMsg, const std::string& chan = u8"") {
        this->data.type = type;
        data.channel = chan;
    }

    R_EXPORT ~Log();

    template<typename Type>
    Log& operator<<(const Type& data) {
        msg += std::to_string(data);
        return (*this);
    }

    template<>
    Log& operator<<(const std::string& data) {
        this->data.msg += data;
        return (*this);
    }

    template<typename Type>
    void append(Type arg) {
        this->data.msg += arg;
    }

    template<>
    void append(const char* data) {
        this->data.msg += data;
    }

};


void    setLogMask(LogTypeFlags enableFlags);
void    setLogChannel(const std::string& channel, B8 enable);

// Display logging info to standard output.
void    enableStandardOutput(B32 enable);
} // Recluse