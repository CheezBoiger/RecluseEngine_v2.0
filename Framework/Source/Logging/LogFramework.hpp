// 
#pragma once

#include "Recluse/Logger.hpp"

#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {


struct LogNode 
{
    struct LogNode* pNext;
    LogMessage      logMessage;
};


class LoggingQueue 
{
public:
    LoggingQueue() 
        : m_pool(nullptr)
        , m_head(nullptr) 
        , m_tail(nullptr)
        , m_mutex(nullptr)
        , m_cursor(0ull)
    {    
    }

    // Initialize/cleanup functions.
    void initialize(U64 maxLogs = 1024ull);
    void cleanup();

    void store(const Log& log);

    // Get the top/head of the queue log. If no logs to display,
    // return null.
    LogMessage* getHead() const;

    // Handle dequeing the head.
    void dequeue();

private:
    // Logging data structure involves a ring buffer based system.
    MemoryPool* m_pool;
    LogNode*    m_head;
    LogNode*    m_tail;
    UPtr     m_cursor;
    void*       m_mutex;
};


class LogDisplay 
{
public:
    LogDisplay(U64 delayNs = 0ull)
        : m_displayThread(nullptr) { }
    ~LogDisplay() { }

    void enable(LogTypeFlags flags);

    void start();
    void stop();

private:
    Thread* m_displayThread;    
};

// Enable operating system color input.
void enableOSColorInput();


namespace LogDatabase {

const LogMessage&   lookup(const std::string& channel, LogType logType);
ResultCode             clear(const std::string& channel, LogType logType);
ResultCode             clearAll();
ResultCode             store(LogMessage& message);
} // LogDatabase
} // Recluse