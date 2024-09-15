//
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

ResultCode TaskProcess::pushTask(TaskPriority priority, Task task)
{
    ScopedLock _(m_tasksMutex);
    auto it = m_tasks.find(priority);
    if (it != m_tasks.end())
    {
        it->second.push_back(task);
    }
    else
    {
        // Add new task into data structure.
        m_tasks[priority] = { task };
    }
    return RecluseResult_Ok;
}


void Application::update()
{
    ResultCode result = onUpdate();
    R_ASSERT(result == RecluseResult_Ok);
}


ResultCode TaskProcess::dispatchTasks()
{
    ScopedLock _(m_tasksMutex);
    if (!m_threadPoolRef)
    {
        // single threaded process.
        for (auto& priorityIt : m_tasks)
        {
            for (auto& task : priorityIt.second)
            {
                ResultCode result = task();
                R_ASSERT(result == RecluseResult_Ok);
            }
        }
    }
    else
    {
        // Multithreaded process. Can utilize multiple threads.
        // Pushes them out to the async workers, and waits until each
        // task in it's priority list is finished.
        for (auto & priorityIt : m_tasks)
        {
            std::vector<U32> ids = { };
            for (auto& task : priorityIt.second)
            {
                AsyncTaskId id = asyncTask(task);
                ids.push_back(id);
            }

            for (auto id : ids)
            {
                waitForTask(id);
            }
        }
    }
    clearTasks();
    return RecluseResult_Ok;
}


void TaskProcess::clearTasks()
{
    // Don't clear the whole priority structure, just the created sets.
    for (auto& taskPrioritySet : m_tasks)
    {
        taskPrioritySet.second.clear();
    }
}


static U32 processTask(void* payload)
{
    R_ASSERT(payload != nullptr);
    TaskProcess* taskProcess = static_cast<TaskProcess*>(payload);

    // Run the task process.
    while (taskProcess->isRunning())
    {
        TaskProcess::OnProcessTask onTask = taskProcess->getOnProcessTask();
        R_ASSERT(onTask != nullptr);
        ResultCode result = onTask(taskProcess);
        // Should clear all tasks regardless.
        if (result == RecluseResult_Ok)
        {
            result = taskProcess->dispatchTasks();
            R_ASSERT(result == RecluseResult_Ok);
        }
    }

    return RecluseResult_Ok;
}


ResultCode TaskProcess::start()
{
    // provide the payload.
    m_thread.payload = (void*)this;
    m_tasksMutex = createMutex("TasksMutex");
    m_asyncCs.initialize();
    m_isRunning = true;
    return createThread(&m_thread, processTask);
}


void TaskProcess::signal(Signal signal)
{ 
    switch (signal)
    {
        case Signal_Stop:
            m_isRunning = false;
            break;
        default:
            break;
    }
}


void TaskProcess::join()
{
    SizeT threadId = getCurrentThreadId();
    if ((m_isRunning == false) && threadId != m_thread.uid)
    {
        joinThread(&m_thread);
    }
}


TaskProcess::AsyncTaskId TaskProcess::asyncTask(Task task)
{
    static TaskProcess::AsyncTaskId id = 0;
    static const TaskProcess::AsyncTaskId InvalidId = -1; // We probably need to prevent wrap around on this value.
    TaskProcess::AsyncTaskId handle = InvalidId;
    {
        ScopedCriticalSection _(m_asyncCs);
        handle = ++id;
        // Wrap around if we manage to increment to the invalid value. By this time, we shouldn't have that many 
        // tasks running on this process.
        if (handle == InvalidId)
            handle = id = 0;
        m_asyncTasks[handle] = { task, false };
    }

    // This function will be the one to run the task.
    auto TaskJobFunction = [&, handle] () -> void 
    {
        m_asyncCs.enter();
        Task asyncTask = m_asyncTasks[handle].task;
        m_asyncCs.leave();

        ResultCode code = asyncTask(); 

        m_asyncCs.enter();
        m_asyncTasks[handle].finished = true;
        m_asyncCs.leave();
    };

    // Submit the task.
    m_threadPoolRef->submitTask(TaskJobFunction);
    return handle;
}


void TaskProcess::waitForTask(TaskProcess::AsyncTaskId taskId)
{
    Bool finished = false;
    // Spinlock until we finish
    while (!finished)
    {
        ScopedCriticalSection _(m_asyncCs);
        auto it = m_asyncTasks.find(taskId);
        if (it != m_asyncTasks.end())
        {
            AsyncTask task = m_asyncTasks[taskId];
            finished = task.finished;
        }
        else
        {
            // There is no task with that id, exit this blocking call.
            break;
        }
    }

    if (finished)
    {
        ScopedCriticalSection _(m_asyncCs);
        m_asyncTasks.erase(taskId);
    }
}


ResultCode Application::makeTaskProcess(TaskProcess::OnProcessTask onProcessTask)
{
    RGUID guid = generateRGUID();
    U64 id = guid.ss.hash0;
    m_taskProcesses[id] = TaskProcess(&m_workerPool, onProcessTask);
    return RecluseResult_Ok;
}


ResultCode Application::startProcesses()
{
    // Start up the assigned tasks.
    for (auto& it : m_taskProcesses)
    {
        it.second.start();
        R_NOTIFY("Application", "Starting process!");
    }
    return RecluseResult_Ok;
}


void Application::stopProcesses()
{
    for (auto& it : m_taskProcesses)
    {
        it.second.signal(TaskProcess::Signal_Stop);
        it.second.join();
    }
}


void Application::startWorkerPool()
{
    m_workerPool.start();
}


void Application::stopWorkerPool()
{
    m_workerPool.stop();
}

namespace MainThreadLoop {

Application* k_pApp         = nullptr;
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
        result = pApp->init(k_pMessageBus);

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
    k_mainLoopInitialized = true;

    return RecluseResult_Ok;
}


ResultCode MainThreadLoop::run()
{
    R_ASSERT(k_pMessageBus      != NULL);
    R_ASSERT(k_pMessageMutex    != MutexValue::kNull);

    while (k_pApp->isRunning()) 
    {
        // All messaging receivers are handled internally by the engine systems.
        // Notify all message receivers.
        {
            ScopedLock lck(k_pMessageMutex);
            k_pMessageBus->notifyAll();
            // Be sure to clear up the bus memory when we finish processing our messages.
            k_pMessageBus->clearQueue();
        }
        // Application update logic is usually here.
        // The application is responsible for handling input, game logic, rendering, physics and whatnot.
        // 
        k_pApp->update();
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