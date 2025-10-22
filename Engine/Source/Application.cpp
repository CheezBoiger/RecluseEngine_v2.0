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

TaskManager::TaskManager()
    : m_tasksMutex(nullptr)
    , m_asyncCs()
{
}

TaskManager::~TaskManager()
{
    cleanUp();
}


ResultCode TaskManager::initialize()
{
    m_tasksMutex = createMutex("TasksMutex");
    m_asyncCs.initialize();
    return RecluseResult_Ok;
}


ResultCode TaskManager::cleanUp()
{
    if (m_tasksMutex) 
        destroyMutex(m_tasksMutex);
        
    if (m_asyncCs.isInitialized())
        m_asyncCs.release();
        
    m_tasksMutex = nullptr;
    return RecluseResult_Ok;
}


ResultCode TaskManager::pushTask(TaskPriority priority, Task task)
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
    ResultCode result = onUpdate(m_taskManager);

    if (result == RecluseResult_Ok)
    {
        result = m_taskManager.dispatchTasks(&m_workerPool);
        R_ASSERT(result == RecluseResult_Ok);
    }

    flushStopRequests();
    flushNewRequests();

    R_ASSERT(result == RecluseResult_Ok);
}


void Application::flushStopRequests()
{
    if (!m_stopRequests.empty())
    {
        std::vector<ProcessId> stoppedProcesses;
        for (auto it : m_stopRequests)
        {
            auto proc = m_taskProcesses.find(it);
            if (proc != m_taskProcesses.end())
            {
                proc->second.signal(TaskProcess::Signal_Stop);
                proc->second.join();   
                stoppedProcesses.push_back(it);
            }
        }

        if (!stoppedProcesses.empty())
        {
            for (auto processId : stoppedProcesses)
                m_taskProcesses.erase(processId);
        }

        m_stopRequests.clear();
    }
}


void Application::flushNewRequests()
{
    if (!m_newRequests.empty())
    {
        for (auto& tuple : m_newRequests)
        {
            ProcessId processId = std::get<2>(tuple);
            m_taskProcesses[processId] = TaskProcess(&m_workerPool, std::get<0>(tuple), std::get<1>(tuple));
            m_taskProcesses[processId].start();
            R_NOTIFY("Application", "Starting process!");
        }
        m_newRequests.clear();
    }
}


Application::ProcessId Application::requestNewProcess(TaskProcess::OnProcessTask onProcessTask, const char* processName)
{
    RGUID guid = generateRGUID();
    ProcessId id = guid.ss.hash0;
    std::tuple<TaskProcess::OnProcessTask, const char*, ProcessId> d = std::make_tuple(onProcessTask, processName, id);
    m_newRequests.push_back(d);
    return id;
}


ResultCode Application::cleanUp()
{
    ResultCode result = onCleanUp();
    m_taskManager.cleanUp();
    if (result == RecluseResult_Ok)
    {
        stopProcesses();
        stopWorkerPool();
        m_initialized = false;
    }
    return result;
}


ResultCode Application::requestStopProcess(ProcessId processId)
{
    m_stopRequests.push_back(processId);
    return RecluseResult_Ok;
}


ResultCode Application::init()
{
    m_taskManager.initialize();
    ResultCode result = onInit();
    if (result == RecluseResult_Ok)
    {
        startWorkerPool();
        startProcesses();
        markInitialized();
        m_isRunning = true;
    }
    return result;
}


ResultCode TaskManager::dispatchTasks(ThreadPool* pool)
{
    ScopedLock _(m_tasksMutex);
    if (!pool)
    {
        // single threaded process. would make our dispatch call syncronous.
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
        for (auto& priorityIt : m_tasks)
        {
            std::vector<U32> ids = { };
            for (auto& task : priorityIt.second)
            {
                AsyncTaskId id = asyncTask(task, pool);
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


void TaskManager::clearTasks()
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
    m_taskManager.initialize();
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


TaskManager::AsyncTaskId TaskManager::asyncTask(Task task, ThreadPool* pool)
{
    static TaskManager::AsyncTaskId id = 0;
    static const TaskManager::AsyncTaskId InvalidId = -1; // We probably need to prevent wrap around on this value.
    TaskManager::AsyncTaskId handle = InvalidId;
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
    pool->submitTask(TaskJobFunction);
    return handle;
}


void TaskManager::waitForTask(TaskManager::AsyncTaskId taskId)
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


Application::ProcessId Application::makeTaskProcess(TaskProcess::OnProcessTask onProcessTask, const char* processName)
{
    RGUID guid = generateRGUID();
    ProcessId processId = guid.ss.hash0;
    m_taskProcesses[processId] = TaskProcess(&m_workerPool, onProcessTask, processName);
    return processId;
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
        result = pApp->init();

    if (result == RecluseResult_Ok)
        k_pApp = pApp;

    return result;
}


ResultCode initialize() 
{
    k_mainLoopInitialized = true;

    return RecluseResult_Ok;
}


ResultCode MainThreadLoop::run()
{
    while (k_pApp->isRunning()) 
    {
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