// 
#pragma once

#include "Core/Logger.hpp"

#include "Core/Threading/Threading.hpp"
#include "Core/Memory/Allocator.hpp"
#include "Core/Memory/MemoryPool.hpp"

namespace Recluse {



class LoggingDatabase {
public:
    void initialize(U64 maxLogs = 200ull) { }

    void store(const Log& log);


private:
    MemoryPool* m_pool;
    Allocator*  m_allocator;
};


class LogDisplay {
public:
    LogDisplay(U64 delayNs = 0ull) { }
    ~LogDisplay() { }

    void enable(LogTypeFlags flags);

    void start();
    void stop();

private:
    Thread* m_displayThread;    
};
} // Recluse