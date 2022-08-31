//
#pragma once

#include "Recluse/Types.hpp"

#include <chrono>
#include <varargs.h>

namespace Recluse {
    

class DateFormatter;

enum LogType 
{
    LogMsg          = 0,
    LogError        = (1 << 0),
    LogDebug        = (1 << 1),
    LogWarn         = (1 << 2),
    LogVerbose      = (1 << 3),
    LogInfo         = (1 << 4),
    LogDontStore    = (1 << 5),
    LogTrace        = (1 << 6),
    LogFatal        = (1 << 7)
};

typedef U32 LogTypeFlags;

struct LogMessage 
{
    std::string msg;
    std::string channel;
    std::string time;       // Optional Time.
    LogType type;
};


enum LogCommand
{
    rEND     = (1 << 0),     // Insert an end line.
    rFLUSH   = (1 << 1),     // Flush out the command.
};

// Logging message struct. Contains message strings to be stored.
// Recommended to keep this off unless there is a need to debug.
struct Log 
{
    // Data message.
    LogMessage data;

    // One time initialize of the data structure for our logging system.
    // Optional message cache size can be defined as well. Be sure to have enough memory if needed.
    static R_PUBLIC_API void initializeLoggingSystem(U32 messageCacheCount = 1024u);
    
    // Final call once the process is completely finished. 
    static R_PUBLIC_API void destroyLoggingSystem();

    Log(LogType type = LogMsg, const std::string& chan = u8"") 
    {
        this->data.type = type;
        data.channel = chan;
    }

    R_PUBLIC_API ~Log();

    template<typename Type>
    Log& operator<<(const Type& data) 
    {
        this->data.msg += std::to_string(data);
        return (*this);
    }

    template<>
    Log& operator<<(const std::string& data) 
    {
        this->data.msg += data;
        return (*this);
    }

    Log& operator<<(const char* data)
    {
        this->data.msg += data;
        return (*this);
    }


    template<typename Type>
    void append(Type arg) 
    {
        this->data.msg += arg;
    }

    template<>
    void append(const char* data) 
    {
        this->data.msg += data;
    }

    R_PUBLIC_API Log& operator<<(const DateFormatter& formatter);
    R_PUBLIC_API Log& operator<<(LogCommand command);

};


R_PUBLIC_API extern void    setLogMask(LogTypeFlags enableFlags);
R_PUBLIC_API extern void    setLogChannel(const std::string& channel, B8 enable);

// Display logging info to standard output.
R_PUBLIC_API extern void    enableStandardOutput(B32 enable);

// Log information into a file.
R_PUBLIC_API extern void        enableLogFile(const std::string& logPath, B32 enable = false);
R_PUBLIC_API extern void        setLogFileMaxCache(U64 maxCachedSizeBytes);
R_PUBLIC_API extern void        setLogFileMaxSize(U64 maxSizeBytes);
R_PUBLIC_API extern std::string getLogFilePath();
} // Recluse