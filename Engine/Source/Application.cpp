//
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Threading/ThreadPool.hpp"

#include "Recluse/Application.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


Application* MainThreadLoop::k_pApp         = nullptr;
Window* MainThreadLoop::k_pWindow           = nullptr;
ThreadPool* MainThreadLoop::k_pThreadPool   = nullptr;
MessageBus* MainThreadLoop::k_pMessageBus   = nullptr;

ErrType MainThreadLoop::loadApp(Application* pApp)
{
    if (!pApp->isInitialized()) {
        pApp->init();
    }
    k_pApp = pApp;
    return REC_RESULT_OK;
}


ErrType MainThreadLoop::initialize() 
{
    k_pMessageBus = new MessageBus();
    k_pMessageBus->initialize();

    return REC_RESULT_OK;
}


ErrType MainThreadLoop::run()
{
    R_ASSERT(k_pWindow      != NULL);
    R_ASSERT(k_pMessageBus  != NULL);

    while (k_pWindow->shouldClose()) {
        RealtimeTick tick = RealtimeTick::getTick();
        pollEvents();
        if (k_pApp) {

            // Update the application tick. Usually game logic is here.
            // This is our sim thread.
            k_pApp->update(tick);
            
        } else {
            R_WARN(__FUNCTION__, "No application loaded to run!");
        }

        // Notify all message receivers.
        k_pMessageBus->notifyAll();
    }

    return REC_RESULT_OK;
}


ErrType MainThreadLoop::cleanUp()
{
    // Clean up the message bus.
    k_pMessageBus->cleanUp();
    delete k_pMessageBus;

    return REC_RESULT_OK;
}


#define LOAD_JOB_THREAD(jobType, flags, thread, jobThreadADT) \
    if (flags & jobType) {                        \
        if (jobThreadADT.find(jobType) != jobThreadADT.end()) \
            jobThreadADT[jobType] = thread; \
    }

ErrType Application::loadJobThread(JobTypeFlags flags, ThreadFunction func)
{
    Thread pThread;
    ErrType result = REC_RESULT_OK;
    result = createThread(&pThread, func);

    if (result == REC_RESULT_OK) {
        m_threads.push_back(pThread);

        LOAD_JOB_THREAD(JOB_TYPE_RENDERER, flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_SIMULATION, flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_AI, flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_ANIMATION, flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_PHYSICS, flags, &m_threads.back(), m_jobThreads);
        LOAD_JOB_THREAD(JOB_TYPE_AUDIO, flags, &m_threads.back(), m_jobThreads);
    }

    return result;
}


Thread* Application::getJobThread(JobType jobType)
{
    if (m_jobThreads.find(jobType) == m_jobThreads.end()) {
        R_WARN("Application", "No job thread available for job type=%d", jobType);
        return nullptr;
    }
    
    return m_jobThreads[jobType];
}


Application* MainThreadLoop::getApp()
{
    return k_pApp;
}


Bool MainThreadLoop::isMainThread()
{
    return getMainThreadId() == getCurrentThreadId();
}


MessageBus* MainThreadLoop::getMessageBus()
{
    R_ASSERT_MSG(k_pMessageBus, "No message bus was initialized! NULL!!");
    return k_pMessageBus;
}
} // Recluse