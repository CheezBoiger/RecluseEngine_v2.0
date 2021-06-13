//
#pragma once

#include "Core/Types.hpp"

#include <chrono>
#include <varargs.h>

namespace Recluse {


enum LogType {
    LogMsg          = 0,
    LogError        = (1 << 0),
    LogDebug        = (1 << 1),
    LogWarn         = (1 << 2),
    LogVerbose      = (1 << 3)
};

typedef U32 LogTypeFlags;

struct Log {
    std::string                                     channel;
    std::string                                     message;
    std::chrono::high_resolution_clock::time_point  timestamp;
    LogType type;

    Log(LogType type, const std::string& chan = u8"") {
        this->type = type;
        channel = chan;
    }

    ~Log();

    template<typename Type>
    Log& operator<<(const Type& data) {
        message += std::to_string(data);
        return (*this);
    }

    template<>
    Log& operator<<(const std::string& data) {
        message += data;
        return (*this);
    }

    template<typename Type>
    void append(Type arg) {
        message += arg;
    }

    template<>
    void append(const char* data) {
        message += data;
    }

};


void setLogMask(LogTypeFlags enableFlags);
void setLogChannel(const std::string& channel, B8 enable);

} // Recluse