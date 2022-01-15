//
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Threading/ThreadPool.hpp"

#include "Recluse/Application.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {

#define LOAD_JOB_THREAD(jobType, flags, thread, jobThreadADT) \
    { \
        if (flags & jobType) \
            if (jobThreadADT.find(jobType) != jobThreadADT.end()) \
                jobThreadADT[jobType] = thread; \
    }

ErrType Application::loadJobThread(JobTypeFlags flags, ThreadFunction func)
{
    Thread pThread;
    ErrType result = REC_RESULT_OK;
    result = createThread(&pThread, func);

    if (result == REC_RESULT_OK) 
    {
        m_threads.push_back(pThread);

        LOAD_JOB_THREAD(JOB_TYPE_RENDERER,      flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_SIMULATION,    flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_AI,            flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_ANIMATION,     flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_PHYSICS,       flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_AUDIO,         flags, &m_threads.back(), m_jobThreads);
    }

    return result;
}


Thread* Application::getJobThread(JobType jobType)
{
    if (m_jobThreads.find(jobType) == m_jobThreads.end()) 
    {
        R_WARN("Application", "No job thread available for job type=%d", jobType);
        return nullptr;
    }
    
    return m_jobThreads[jobType];
}

namespace MainThreadLoop {

Application* k_pApp         = nullptr;
Window* k_pWindow           = nullptr;
ThreadPool* k_pThreadPool   = nullptr;
MessageBus* k_pMessageBus   = nullptr;
Mutex k_pMessageMutex       = MutexValue::kNull;
F32 k_fixedTickRateSeconds  = 1.0f / 60.0f;

ErrType loadApp(Application* pApp)
{
    ErrType result = REC_RESULT_OK;

    if (!pApp->isInitialized())
        result = pApp->init();

    if (result == REC_RESULT_OK)
        k_pApp = pApp;

    return result;
}


ErrType initialize() 
{
    R_ASSERT(k_pMessageMutex == MutexValue::kNull);

    k_pMessageMutex = createMutex();
    k_pMessageBus = new MessageBus();
    k_pMessageBus->initialize();

    return REC_RESULT_OK;
}


ErrType MainThreadLoop::run()
{
    R_ASSERT(k_pWindow          != NULL);
    R_ASSERT(k_pMessageBus      != NULL);
    R_ASSERT(k_pMessageMutex    != MutexValue::kNull);

    while (k_pWindow->shouldClose()) 
    {
        RealtimeTick tick = RealtimeTick::getTick(0);
        pollEvents();
        if (k_pApp) 
        {
            // Update the application tick. Usually game logic is here.
            // This is our sim thread.
            k_pApp->update(tick);   
        } 
        else 
        {
            R_WARN(__FUNCTION__, "No application loaded to run!");
        }

        // Notify all message receivers.
        ScopedLock lck(k_pMessageMutex);
        k_pMessageBus->notifyAll();
    }

    return REC_RESULT_OK;
}


ErrType cleanUp()
{
    destroyMutex(k_pMessageMutex);
    k_pMessageMutex = MutexValue::kNull;

    // Clean up the message bus.
    k_pMessageBus->cleanUp();
    delete k_pMessageBus;

    return REC_RESULT_OK;
}



Application* getApp()
{
    return k_pApp;
}


Bool isMainThread()
{
    return getMainThreadId() == getCurrentThreadId();
}


MessageBus* getMessageBus()
{
    R_ASSERT_MSG(k_pMessageBus, "No message bus was initialized! NULL!!");
    return k_pMessageBus;
}


F32 getFixedTickRate()
{
    return k_fixedTickRateSeconds;
}


void setFixedTickRate(F32 tickRateSeconds)
{
    k_fixedTickRateSeconds = tickRateSeconds;
}
} // MainThreadLoop
} // Recluse