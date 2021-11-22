//
#include "Recluse/Threading/ThreadPool.hpp"

namespace Recluse {

ThreadPool::ThreadPool(U32 numWorkers)
    : m_allWorkersFinished(true)
{
    m_threadWorkers.resize(numWorkers);
}


ThreadPool::~ThreadPool()
{
    // Need to clean up.s
}
} // Recluse