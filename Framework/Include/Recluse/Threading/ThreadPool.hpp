//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Threading/Sema.hpp"

#include <vector>
#include <list>
#include <functional>

namespace Recluse {


typedef std::function<void()> ThreadTask;

class R_PUBLIC_API ThreadPool 
{
public:
    enum Status 
    {
        Status_Idle,
        Status_Running,
        Status_Pause,
        Status_Stopping,
        Status_Stopped
    };

    enum Signal 
    {
        Signal_None,
        Signal_Stop = (1 << 0),
        Signal_Pause = (1<<1),
        Signal_Resume =(1<<2)
    };

    ThreadPool(U32 numWorkers = 2);
    ~ThreadPool();

    ResultCode submitTask(ThreadTask job);
    
    void start();
    void stop();

    //ResultCode wait();

    //Bool isExecuting();
    
private:
    // The actual worker itself.
    struct Worker
    {
        Worker(ThreadPool* pool = nullptr)
            : poolRef(pool)
            , signals(0)
            , status(Status_Stopped) { thread = { }; }

        Thread  thread;
        Status  status;

        ThreadTask  nextTask();
        void        signal(Signal signal) { signals |= signal; }
        U32         getSignals() { return signals; }
        void        join();
        void        clearSignals() { signals = 0; }

    private:
        U32     signals;
        ThreadPool* poolRef;
    };

    static U32 threadEntryTask(void* payload);

    // Tasks to complete, which are carried by worker threads.
    CriticalSection                     m_taskCs;

    std::list<ThreadTask>               m_jobTasks;
    std::vector<Worker>                 m_threadWorkers;
};
} // Recluse