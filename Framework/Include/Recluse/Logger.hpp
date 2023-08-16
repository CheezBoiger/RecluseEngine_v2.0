//
#pragma once

#include "Recluse/Types.hpp"

#include <chrono>
#include <varargs.h>

namespace Recluse {
    

class DateFormatter;

namespace Math {
struct Float3;
struct Matrix22;
struct Matrix33;
struct Matrix44;
struct Matrix43;
struct Float2;
struct Float4;
} // Math

enum LogType 
{
    LogType_Msg          = 0,
    LogType_Error        = (1 << 0),
    LogType_Debug        = (1 << 1),
    LogType_Warn         = (1 << 2),
    LogType_Verbose      = (1 << 3),
    LogType_Info         = (1 << 4),
    LogType_DontStore    = (1 << 5),
    LogType_Trace        = (1 << 6),
    LogType_Fatal        = (1 << 7),
    LogType_Notify       = (1 << 8)
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
private:
    R_PUBLIC_API void stringify(const Math::Float2& f2);
    R_PUBLIC_API void stringify(const Math::Float3& f3);
    R_PUBLIC_API void stringify(const Math::Float4& f4);
    R_PUBLIC_API void stringify(const Math::Matrix22& m22);
    R_PUBLIC_API void stringify(const Math::Matrix33& m33);
    R_PUBLIC_API void stringify(const Math::Matrix44& m44);
    R_PUBLIC_API void stringify(const Math::Matrix43& m43);
public:
    // Data message.
    LogMessage data;

    // One time initialize of the data structure for our logging system.
    // Optional message cache size can be defined as well. Be sure to have enough memory if needed.
    static R_PUBLIC_API void initializeLoggingSystem(U32 messageCacheCount = 1024u);
    
    // Final call once the process is completely finished. 
    static R_PUBLIC_API void destroyLoggingSystem();

    Log(LogType type = LogType_Msg, const std::string& chan = u8"") 
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

    template<>
    Log& operator<<(const Math::Float2& f2)
    {
        stringify(f2);
        return (*this);
    }

    template<>
    Log& operator<<(const Math::Float3& f3)
    {
        stringify(f3);
        return (*this);
    }

    template<>
    Log& operator<<(const Math::Float4& f4)
    {
        stringify(f4);
        return (*this);
    }

    template<>
    Log& operator<<(const Math::Matrix22& m22)
    {
        stringify(m22);
        return (*this);
    }

    template<>
    Log& operator<<(const Math::Matrix33& m33)
    {
        stringify(m33);
        return (*this);
    }

    template<>
    Log& operator<<(const Math::Matrix44& m44)
    {
        stringify(m44);
        return (*this);
    }

    template<>
    Log& operator<<(const Math::Matrix43& m43)
    {
        stringify(m43);
        return (*this);
    }

    R_PUBLIC_API Log& operator<<(const DateFormatter& formatter);
    R_PUBLIC_API Log& operator<<(LogCommand command);
};


// Filter out any log types we want to listen to. By default, all log types are enabled.
R_PUBLIC_API extern void        setLogMask(LogTypeFlags enableFlags);
R_PUBLIC_API extern void        enableLogTypes(LogTypeFlags flags);
R_PUBLIC_API extern void        disableLogTypes(LogTypeFlags flags);

// Show which channels to filter out. By default, all channels are enabled, so show which channels 
// we wish to store.
R_PUBLIC_API extern void        setLogChannel(const std::string& channel, B8 enable);

// Display logging info to standard output.
R_PUBLIC_API extern void        enableStandardOutput(B32 enable);

// Log information into a file.
R_PUBLIC_API extern void        enableLogFile(const std::string& logPath, B32 enable = false);
R_PUBLIC_API extern void        setLogFileMaxCache(U64 maxCachedSizeBytes);
R_PUBLIC_API extern void        setLogFileMaxSize(U64 maxSizeBytes);
R_PUBLIC_API extern std::string getLogFilePath();
} // Recluse