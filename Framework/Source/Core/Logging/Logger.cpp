// 
#include "Core/Logger.hpp"

#include "Core/Messaging.hpp"
#include "Core/Logging/LogFramework.hpp"

namespace Recluse {


static LoggingQueue*    loggingQueue;
static Thread           displayThread;
static volatile B32     isLogging           = true;


static void printLog(const Log* log)
{
    printf("%s: %s\n", log->channel.c_str(), log->message.c_str());
}


ErrType displayFunction(void* data)
{
    while (isLogging) {

        Log* pLog = nullptr;
        pLog = loggingQueue->getHead();

        if (pLog) {

            // TODO: Include the timestamp as well.
            printLog(pLog);
            // Remove the log
            loggingQueue->dequeue();

        }
    }

    // Check last time if there are any more logs to display before dropping out.
    while (loggingQueue->getHead() != NULL) {
        Log* pLog = loggingQueue->getHead();
        printLog(pLog);
        loggingQueue->dequeue();
    }

    return REC_RESULT_OK;
}

Log::~Log()
{
    if (loggingQueue && !(type & LogDontStore)) {

        loggingQueue->store(*this);

    }

}


void LoggingQueue::store(const Log& log)
{
    SizeT alignedSzBytes = sizeof(LogNode);
    alignedSzBytes = RECLUSE_ALLOC_MASK(alignedSzBytes, ARCH_PTR_SZ_BYTES);
    SizeT poolSzBytes = m_pool->getTotalSizeBytes();

    lockMutex(m_mutex);

    if (!m_head) {
        SizeT newCursor = m_cursor;
    
        if (newCursor >= (m_pool->getBaseAddress() + poolSzBytes)) {
            newCursor = m_pool->getBaseAddress();
        }

        m_head = (LogNode*)m_cursor;
        new (m_head) LogNode;

        m_tail = m_head;
        m_head->logMessage = log;
        m_head->pNext = nullptr;

        // Cursor should be after tail.
        m_cursor = newCursor + alignedSzBytes;

    } else {

        PtrType addrHead = (PtrType)m_head;
        PtrType temp = m_cursor + alignedSzBytes;

        if (m_cursor != addrHead) {
            LogNode* newNode = (LogNode*)m_cursor;
            new (newNode) LogNode;
  
            newNode->logMessage = log;
            newNode->pNext = nullptr;

            m_tail->pNext = newNode;
            m_tail = newNode;          
            
            m_cursor = temp;

            if (m_cursor >= (m_pool->getBaseAddress() + poolSzBytes)) {
            
                m_cursor = m_pool->getBaseAddress();

            }
        }

    }

    unlockMutex(m_mutex);
}


void LoggingQueue::dequeue()
{
    lockMutex(m_mutex);

    if (m_tail != m_head) {

        m_head->logMessage.type = LogDontStore;
        LogNode* node = m_head;
        m_head = m_head->pNext;
        node->~LogNode();

    } else {

        m_tail = nullptr;
        m_head = nullptr;

    }

    unlockMutex(m_mutex);
}



void LoggingQueue::initialize(U64 maxLogs)
{
    m_mutex = createMutex();

    // TODO: Not sure if we want to allocate our logging queue on heap...
    m_pool = new MemoryPool(sizeof(LogNode) * maxLogs);
    m_cursor = m_pool->getBaseAddress();
}


void LoggingQueue::cleanup()
{
    if (m_mutex) {
    
        destroyMutex(m_mutex);
    
        m_mutex = nullptr;
    
    }

    if (m_pool) {
        delete m_pool;
        m_pool = nullptr;
    }
}


void Log::initializeLoggingSystem()
{
    if (!loggingQueue) {
        loggingQueue = rlsMalloc<LoggingQueue>(sizeof(LoggingQueue));
        loggingQueue->initialize();
    }

    isLogging = true;

    createThread(&displayThread, displayFunction);
}


void Log::destroyLoggingSystem()
{
    isLogging = false;
    joinThread(&displayThread);

    if (loggingQueue) {

        loggingQueue->cleanup();

        rlsFree<LoggingQueue>(loggingQueue);
        loggingQueue = nullptr;   

    }

}


Log* LoggingQueue::getHead() const
{
    Log* pLog = nullptr;

    lockMutex(m_mutex);

    pLog = (m_head) ? &m_head->logMessage : nullptr;

    unlockMutex(m_mutex);

    return pLog;    
}
} // Recluse