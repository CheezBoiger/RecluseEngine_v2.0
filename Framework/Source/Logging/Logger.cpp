// 
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/DateTime.hpp"
#include "Logging/LogFramework.hpp"

#include "Recluse/Filesystem/Archive.hpp"

namespace Recluse {

const char* DateFormatter::kDefaultFormat = "%Y-%M-%D %h:%m:%s";

static LoggingQueue*    loggingQueue = nullptr;
static Thread           displayThread;
static volatile B32     isLogging           = true;

// TODO: Check if these colors work for most windows machines.
// TODO: Maybe a binary semaphore would be better, to order the display process?

#define R_COLOR_RED     "\x1b[31m"
#define R_COLOR_GREEN   "\x1b[32m"
#define R_COLOR_YELLOW  "\x1b[33m"
#define R_COLOR_BLUE    "\x1b[34m"
#define R_COLOR_MAGENTA "\x1b[35m"
#define R_COLOR_CYAN    "\x1b[36m"

#define R_COLOR_BRIGHT  "\x1b[1m"
#define R_COLOR_RESET   "\x1b[0m"

static void printLog(const LogMessage* log)
{
    char* color     = R_COLOR_RESET;
    char* logStr    = "";

    switch (log->type) 
    {
        case LogWarn:       color = R_COLOR_YELLOW;     logStr = "Warn"; break;
        case LogDebug:      color = R_COLOR_MAGENTA;    logStr = "Debug"; break;
        case LogVerbose:    color = R_COLOR_CYAN;       logStr = "Verbose"; break;
        case LogError:      color = R_COLOR_RED;        logStr = "Error"; break;        
        case LogTrace:      color = R_COLOR_GREEN;      logStr = "Trace"; break;
        case LogInfo:                                   logStr = "Info"; break;
        case LogFatal:      color = R_COLOR_RED;        logStr = "FATAL"; break;
        case LogMsg:
        default: break;
    }

    if (!log->time.empty())
    {
        printf
            (
                "%s " R_COLOR_RESET "[%s%s" R_COLOR_RESET  "][" "%s%s" R_COLOR_RESET "] %s: %s%s" R_COLOR_RESET "\n",
                color,
                color,
                logStr,
                color,
                log->time.c_str(),
                log->channel.c_str(),
                color,
                log->msg.c_str()
            );
    }
    else
    {
        printf
            (
                "%s " R_COLOR_RESET "[%s%s" R_COLOR_RESET  "] %s: %s%s" R_COLOR_RESET "\n",
                color,
                color,
                logStr,
                log->channel.c_str(),
                color,
                log->msg.c_str()
            );
    }
}


static ErrType displayFunction(void* data)
{
    (void)data;

    while (isLogging) 
    {
        LogMessage* pLog    = nullptr;
        pLog                = loggingQueue->getHead();

        if (pLog) 
        {
            // TODO: Include the timestamp as well.
            printLog(pLog);
            // Remove the log
            loggingQueue->dequeue();
        }
    }

    // Check last time if there are any more logs to display before dropping out.
    while (loggingQueue->getHead() != NULL) 
    {
        LogMessage* pLog = loggingQueue->getHead();
        printLog(pLog);
        loggingQueue->dequeue();
    }

    return R_RESULT_OK;
}

Log::~Log()
{
    if (loggingQueue && isLogging) 
    {
        loggingQueue->store(*this);
    }
}


void LoggingQueue::store(const Log& log)
{
    SizeT alignedSzBytes    = sizeof(LogNode);
    alignedSzBytes          = R_ALLOC_MASK(alignedSzBytes, ARCH_PTR_SZ_BYTES);
    SizeT poolSzBytes       = m_pool->getTotalSizeBytes();

    ScopedLock lck(m_mutex);

    if (!m_head) 
    {
        SizeT newCursor = m_cursor;
    
        if ((newCursor + alignedSzBytes) >= (m_pool->getBaseAddress() + poolSzBytes)) 
        {
            newCursor   = m_pool->getBaseAddress();
            m_cursor    = newCursor;

        }

        m_head = (LogNode*)m_cursor;
        m_head->~LogNode();
        new (m_head) LogNode;

        m_tail              = m_head;
        m_head->logMessage  = std::move(log.data);
        m_head->pNext       = nullptr;

        // Cursor should be after tail.
        m_cursor            = newCursor + alignedSzBytes;
    } 
    else 
    {
        PtrType addrHead    = (PtrType)m_head;
        PtrType temp        = m_cursor + alignedSzBytes;

        if (temp >= (m_pool->getBaseAddress() + poolSzBytes)) 
        {
            m_cursor    = m_pool->getBaseAddress();
            temp        = m_cursor + alignedSzBytes;
        }

        if (m_cursor != addrHead) 
        {
            LogNode* newNode = (LogNode*)m_cursor;
            newNode->~LogNode();
            new (newNode) LogNode;
  
            newNode->logMessage = std::move(log.data);
            newNode->pNext      = nullptr;

            m_tail->pNext       = newNode;
            m_tail              = newNode;          
            
            m_cursor            = temp;
        }
    }
}


void LoggingQueue::dequeue()
{
    ScopedLock scoped(m_mutex);

    if (m_tail != m_head) 
    {
        m_head->logMessage.type = LogDontStore;
        LogNode* node           = m_head;
        m_head                  = m_head->pNext;
        node->~LogNode();
    } 
    else 
    {
        m_tail = nullptr;
        m_head = nullptr;
    }
}



void LoggingQueue::initialize(U64 maxLogs)
{
    m_mutex = createMutex();

    U64 szTotalBytes    = R_ALLOC_MASK((sizeof(LogNode) * maxLogs), ARCH_PTR_SZ_BYTES);
    U64 szLogNode       = R_ALLOC_MASK(sizeof(LogNode), ARCH_PTR_SZ_BYTES);
    // TODO: Not sure if we want to allocate our logging queue on heap...
    m_pool              = new MemoryPool(szTotalBytes);

    // Preinitialize the ring buffer full of logs. This must be cleaned up due to separate string
    // initialization.
    for (U64 i = 0; i < m_pool->getTotalSizeBytes(); i += szLogNode) 
    {
        U64 addr        = m_pool->getBaseAddress() + i;
        LogNode* node   = (LogNode*)addr;
        new (node) LogNode();
    }

    m_cursor = m_pool->getBaseAddress();
}


void LoggingQueue::cleanup()
{
    if (m_mutex) 
    {
        destroyMutex(m_mutex);
    
        m_mutex = nullptr;
    }

    if (m_pool) 
    {
        // We need to individually delete all LogNodes, due to strings being allocated separately.
        // Without this, we will have memory leaks.
        const U64 szLogNode = R_ALLOC_MASK(sizeof(LogNode), ARCH_PTR_SZ_BYTES);
        for (U64 i = 0; i < m_pool->getTotalSizeBytes(); i += szLogNode)
        {
            U64 addr = m_pool->getBaseAddress() + i;
            LogNode* node = (LogNode*)addr;
            node->~LogNode();
        }

        delete m_pool;
        m_pool = nullptr;
    }
}


void Log::initializeLoggingSystem(U32 messageCacheCount)
{
    if (!loggingQueue) 
    {
        loggingQueue = rlsMalloc<LoggingQueue>(sizeof(LoggingQueue));
        loggingQueue->initialize(static_cast<U32>(messageCacheCount));
    }

    isLogging = true;

    enableOSColorInput();

    createThread(&displayThread, displayFunction);
}


void Log::destroyLoggingSystem()
{
    isLogging = false;
    joinThread(&displayThread);

    if (loggingQueue) 
    {
        loggingQueue->cleanup();

        rlsFree<LoggingQueue>(loggingQueue);
        loggingQueue = nullptr;   
    }

}


LogMessage* LoggingQueue::getHead() const
{
    LogMessage* pLog = nullptr;

    ScopedLock lck(m_mutex);

    pLog = (m_head) ? &m_head->logMessage : nullptr;
    return pLog;    
}


Log& Log::operator<<(const DateFormatter& formatter)
{
    data.time = formatter.getFormattedString();
    return (*this);
}


Log& Log::operator<<(LogCommand command)
{
    if (command & rEND)
    {
        data.msg += "\n";
    }

    // We are given the command to flush out this message data.
    // After flush, we must clear out the existing buffer.
    if (command & rFLUSH)
    {
        loggingQueue->store(*this);
        data.msg.clear();
        data.time.clear();
    }

    return (*this);
}
} // Recluse