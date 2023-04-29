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

ResultCode Application::loadJobThread(JobTypeFlags flags, ThreadFunction func)
{
    Thread pThread;
    ResultCode result = RecluseResult_Ok;
    result = createThread(&pThread, func);

    if (result == RecluseResult_Ok) 
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


void Application::update(const RealtimeTick& tick)
{
}

namespace MainThreadLoop {

Application* k_pApp         = nullptr;
Window* k_pWindow           = nullptr;
ThreadPool* k_pThreadPool   = nullptr;
MessageBus* k_pMessageBus   = nullptr;
Mutex k_pMessageMutex       = MutexValue::kNull;
F32 k_fixedTickRateSeconds  = 1.0f / 60.0f;
Bool k_mainLoopInitialized  = false;

ResultCode loadApp(Application* pApp)
{
    R_ASSERT_FORMAT
        (
            k_mainLoopInitialized, 
            "Main Loop must be initialized first before calling this function!"
        );

    ResultCode result = RecluseResult_Ok;

    if (!pApp->isInitialized())
        result = pApp->init(k_pWindow, k_pMessageBus);

    if (result == RecluseResult_Ok)
        k_pApp = pApp;

    return result;
}


ResultCode initialize() 
{
    R_ASSERT(k_pMessageMutex == MutexValue::kNull);

    k_pMessageMutex = createMutex();
    k_pMessageBus = new MessageBus();
    k_pMessageBus->initialize();

    k_pWindow = Window::create(u8"TestApp", 0, 0, 800, 600);
    k_pWindow->open();

    k_mainLoopInitialized = true;

    // initialize the main watch.
    RealtimeTick::initializeWatch(getMainThreadId(), JOB_TYPE_MAIN);    

    return RecluseResult_Ok;
}


ResultCode MainThreadLoop::run()
{
    R_ASSERT(k_pWindow          != NULL);
    R_ASSERT(k_pMessageBus      != NULL);
    R_ASSERT(k_pMessageMutex    != MutexValue::kNull);

    while (!k_pWindow->shouldClose()) 
    {
        RealtimeTick::updateWatch(getCurrentThreadId(), JOB_TYPE_MAIN);
        RealtimeTick tick = RealtimeTick::getTick(JOB_TYPE_MAIN);
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
        // Be sure to clear up the bus memory when we finish processing our messages.
        k_pMessageBus->clearQueue();
    }

    return RecluseResult_Ok;
}


ResultCode cleanUp()
{
    ResultCode result = RecluseResult_Ok; 

    if (k_pApp)
    {
        result = k_pApp->cleanUp();
    }

    Window::destroy(k_pWindow);
    k_pWindow = nullptr;

    destroyMutex(k_pMessageMutex);
    k_pMessageMutex = MutexValue::kNull;

    // Clean up the message bus.
    k_pMessageBus->cleanUp();
    delete k_pMessageBus;

    k_mainLoopInitialized = false;
    return result;
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
    R_ASSERT_FORMAT(k_pMessageBus, "No message bus was initialized! NULL!!");
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