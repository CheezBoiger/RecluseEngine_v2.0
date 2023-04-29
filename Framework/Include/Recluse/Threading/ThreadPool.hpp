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


struct TaskJob
{
    ThreadJob   jobFunc;
    void*       pPayload;
};

class ThreadPool 
{
public:
    R_PUBLIC_API ThreadPool(U32 numWorkers = 2);
    R_PUBLIC_API ~ThreadPool();

    R_PUBLIC_API ResultCode submitJob(ThreadJob job);
    
    R_PUBLIC_API ResultCode waitFinished();

    R_PUBLIC_API Bool isExecuting();

    R_PUBLIC_API Bool isFinished();
    
private:

    void threadStartFunc()
    {
        while (!m_allWorkersFinished)
        {
            if (tryLockMutex(m_mutex) == RecluseResult_Ok)
            {
                TaskJob taskJob = m_jobTasks.back();
                
                m_jobTasks.pop_back();
                // Unlock the mutex when we finish.
                unlockMutex(m_mutex);

                // Run the task with the given payload.
                taskJob.jobFunc(taskJob.pPayload);

                JobFinishedPayload finished = { };
                finished.func = taskJob.jobFunc;
                finished.resultCode = 00;

                while (!tryLockMutex(m_finishedMutex))
                {
                    // spin until we get our mutex.
                }

                m_finishedResults.push_back(finished);
                unlockMutex(m_finishedMutex);
            }
        }
    }

    // Tasks to complete, which are carried by worker threads.
    std::vector<TaskJob>                m_jobTasks;
    std::vector<JobFinishedPayload>     m_finishedResults;

    std::vector<Thread>                 m_threadWorkers;
    std::vector<Thread>                 m_finishedThreads;
    Bool                                m_allWorkersFinished;
    Mutex                               m_mutex;
    Mutex                               m_finishedMutex;
};
} // Recluse