//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Threading/Sema.hpp"

#include <vector>

namespace Recluse {


typedef ThreadFunction ThreadJob;


struct R_PUBLIC_API JobFinishedPayload 
{
    ThreadJob   func;
    SizeT       uid;
    U32         resultCode;
};

class ThreadPool 
{
public:
    R_PUBLIC_API ThreadPool(U32 numWorkers = 2);
    R_PUBLIC_API ~ThreadPool();

    R_PUBLIC_API ErrType submitJob(ThreadJob job);
    R_PUBLIC_API ErrType execute();
    
    R_PUBLIC_API ErrType waitFinished();

    R_PUBLIC_API Bool isExecuting();

    R_PUBLIC_API Bool isFinished();
    
private:

    // Tasks to complete, which are carried by worker threads.
    std::vector<ThreadJob> m_jobTasks;
    std::vector<JobFinishedPayload> m_finishedResults;

    std::vector<Thread> m_threadWorkers;
    std::vector<Thread> m_finishedThreads;

    Bool m_allWorkersFinished;
};
} // Recluse