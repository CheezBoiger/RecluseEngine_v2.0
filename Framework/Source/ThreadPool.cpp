//
#include "Recluse/Threading/ThreadPool.hpp"

namespace Recluse {


U32 ThreadPool::threadEntryTask(void* payload)
{
    // Entry point of a worker thread, that begins to 
    // query for any tasks it needs to do.
    Worker* worker = static_cast<Worker*>(payload);
    if (!worker)
    {
        return RecluseResult_Failed;
    }

    worker->status = ThreadPool::Status_Running;

    // Check and loop while we aren't signaled to stop.
    while (!(worker->getSignals() & ThreadPool::Signal_Stop))
    {
        // Try to pull the next task.
        ThreadTask task = worker->nextTask();
        if (task)
        {
            worker->status = ThreadPool::Status_Running;

            task();
        }
        else
        {
            worker->status = ThreadPool::Status_Idle;
        }
    }

    worker->status = ThreadPool::Status_Stopped;
    return RecluseResult_Ok;
}

ThreadPool::ThreadPool(U32 numWorkers)
{
    m_threadWorkers.resize(numWorkers);
    m_taskCs.initialize();
}


ThreadPool::~ThreadPool()
{
    // Need to clean up.
    stop();
    m_taskCs.release();
}


void ThreadPool::start()
{
    for (auto& worker : m_threadWorkers)
    {
        worker = Worker(this);
        // Pass on this thread pool.
        worker.thread.payload = (void*)&worker;
        createThread(&worker.thread, threadEntryTask);
    }
}


void ThreadPool::stop()
{
    for (auto& worker : m_threadWorkers)
    {
        worker.signal(ThreadPool::Signal_Stop);
        worker.join();
    }
}


ResultCode ThreadPool::submitTask(ThreadTask job)
{
    ScopedCriticalSection _(m_taskCs);
    m_jobTasks.push_back(job);
    return RecluseResult_Ok;
}


ThreadTask ThreadPool::Worker::nextTask()
{
    ThreadTask task = nullptr;
    if (poolRef)
    {
        // Pick up the next front task, and remove from the queue.
        ScopedCriticalSection _(poolRef->m_taskCs);
        if (!poolRef->m_jobTasks.empty())
        {
            task = poolRef->m_jobTasks.front();
            poolRef->m_jobTasks.pop_front();
        }
    }
    return task;
}


void ThreadPool::Worker::join()
{
    joinThread(&thread);
}
} // Recluse