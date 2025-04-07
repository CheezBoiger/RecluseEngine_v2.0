//
#pragma once
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Scene/SceneLoader.hpp"

#include "Recluse/Game/GameSystem.hpp"

#include "Recluse/Time.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Threading/ThreadPool.hpp"

#include "Recluse/MessageBus.hpp"

#include "RecluseEngine_exports.hpp"

#include <map>
#include <list>
#include <set>
#include <functional>
#include <tuple>

#define R_BUILD_RETAIL                (0)
#define R_BUILD_RELEASE               (0)
#define R_BUILD_DEBUG                 (0)
#define R_BUILD_DEVELOPER             (0)

#define R_CLIENT                (1 << 0)
#define R_SERVER                (1 << 1)
#define R_CLIENT_AND_SERVER     (R_CLIENT | R_SERVER)

#define R_NET_TYPE              (R_CLIENT)

namespace Recluse {

class Application;

typedef std::function<ResultCode()> Task;
typedef U32 TaskPriority;

typedef U32 TaskTypeFlags;


// Task manager that runs and schedules tasks based on priority.
// Uses current thread pool to handle and run certain tasks.
class RecluseEngine_PUBLIC_API TaskManager
{
public:
    typedef U32 AsyncTaskId;

    TaskManager();
    ~TaskManager();

    ResultCode      initialize();
    ResultCode      cleanUp();

    // Push a task with the given priorities.
    // Likely want this to be pushed during update call, so that thread pool can take hold.
    // Parallel tasks are done when they are pushed with the same priority.
    // 0 is highest priority, with least priority values going up.
    ResultCode      pushTask(TaskPriority priority, Task task);

    // Asyncronous task, that does not run in parallel like the pushTask().
    // This task works without barriers, and should work separately.
    AsyncTaskId     asyncTask(Task task, ThreadPool* pool);
    void            waitForTask(AsyncTaskId taskId);

    // Dispatch all pushed tasks that were called with pushTask(). 
    // Ensure any data within scope, should be called with this manually in the scope of that data to be processed.
    // Failure to do so will result in undefined behaviour, likely a crash.
    ResultCode      dispatchTasks(ThreadPool* pool);
    void            clearTasks();

private:
    struct AsyncTask
    {
        Task task;
        Bool finished;
    };
    Mutex                                       m_tasksMutex;
    CriticalSection                             m_asyncCs;
    std::map<TaskPriority, std::vector<Task>>   m_tasks;
    std::map<AsyncTaskId, AsyncTask>            m_asyncTasks;
};


// Task process is a separate asyncronous process, that runs independent of the main thread.
// This would need to be used for anything that requires it's own independent execution.
class RecluseEngine_PUBLIC_API TaskProcess
{

public:
    enum Signal 
    {
        Signal_Notify,  //< Signal to notify the process in it's task.
        Signal_Stop,    //< Signal to stop the process.
        Signal_Pause,   //< Signal to pause the process.
        Signal_Resume   //< Signal to resume the process.
    };

    typedef ThreadFunction ProcessTask;
    typedef std::function<ResultCode(TaskProcess*)> OnProcessTask; 

    TaskProcess(ThreadPool* workerPool = nullptr, OnProcessTask onTask = nullptr, const char* processName = nullptr)
        : m_onTask(onTask)
        , m_threadPoolRef(workerPool)
        , m_processName(processName ? processName : "")
        , m_isRunning(false) { }

    ~TaskProcess() { }

    // Start the process.
    ResultCode                  start();

    // Push a task with the given priorities.
    // Likely want this to be pushed during update call, so that thread pool can take hold.
    // Parallel tasks are done when they are pushed with the same priority.
    // 0 is highest priority, with least priority values going up.
    ResultCode                  pushTask(TaskPriority priority, Task task) { return m_taskManager.pushTask(priority, task); }

    // Asyncronous task, that does not run in parallel like the pushTask().
    // This task works without barriers, and should work separately.
    TaskManager::AsyncTaskId    asyncTask(Task task) { return m_taskManager.asyncTask(task, m_threadPoolRef); }
    void                        waitForTask(TaskManager::AsyncTaskId taskId) { m_taskManager.waitForTask(taskId); }

    // Get the process task that is running on this Process.
    OnProcessTask               getOnProcessTask() { return m_onTask; }

    // Check if this process is running.
    Bool                        isRunning() const  { return m_isRunning; }

    // Signal to the process. Can be called by the main task.
    void                        signal(Signal signal = Signal_Notify);

    // Dispatch all pushed tasks that were called with pushTask(). 
    // Ensure any data within scope, should be called with this manually in the scope of that data to be processed.
    // Failure to do so will result in undefined behaviour, likely a crash.
    ResultCode                  dispatchTasks() { return m_taskManager.dispatchTasks(m_threadPoolRef); }
    void                        clearTasks() { m_taskManager.clearTasks(); }

    std::string                 getProcessName() const { return m_processName; }
    TaskManager&                getTaskManager() { return m_taskManager; }

    // Waits to join back with the caller thread. Will block the caller until this process is complete.
    // Will not attempt to join, if the process itself attempts to call this.
    void                        join();

private:

    TaskManager                                 m_taskManager;
    Bool                                        m_isRunning;
    Thread                                      m_thread;

    ProcessTask                                 m_mainTask;
    OnProcessTask                               m_onTask;
    ThreadPool*                                 m_threadPoolRef;
    std::string                                 m_processName;
};

// Application interface for your application.
// This should, and would be integrated into your game, in order to 
// connect to the engine components, as well as the editor system.
class RecluseEngine_PUBLIC_API Application 
{
public:
    typedef U64 ProcessId;
    static const ProcessId InvalidProcessId = 0;

    Application(const std::string& appName = "")
        : m_appName(appName)
        , m_pScene(nullptr)
        , m_pMessageBusRef(nullptr)
        , m_initialized(false)
        , m_isRunning(false)
    { }

    virtual                 ~Application() { }    

    // System update.
    void                    update();

    ResultCode              cleanUp();
    ResultCode              init(MessageBus* pMessageBus);

    Engine::Scene*          getScene() { return m_pScene; }
    MessageBus*             getMessageBus() { return m_pMessageBusRef; }

    inline Bool             isInitialized() const { return m_initialized; }
    inline Bool             isRunning() const { return m_isRunning; }

    void                    stop() { m_isRunning = false; }

    // Creates a task process, and requests for one to be made. This process
    // runs asyncronously, independent of the main thread.
    ProcessId               makeTaskProcess(TaskProcess::OnProcessTask onProcessTask, const char* processName = nullptr);

    // Requests to stop a process task, this would not be completed until next tick update.
    ResultCode              requestStopProcess(ProcessId processId);

    // Requests to start a new process task, this will start at the end of the tick update.
    ProcessId               requestNewProcess(TaskProcess::OnProcessTask onProcessTask, const char* processName = nullptr);

protected:
    //! Application specific initialization. This requires 
    //! individual app owners to initialize each module for their 
    //! game system.
    virtual ResultCode      onInit() = 0;
    
    //! Like onInit(), cleans up all application defined resources.
    //! Requires all modules initialized, to be cleaned up manually as well.
    virtual ResultCode      onCleanUp() = 0;

    //! User logic for updates and task creation.
    virtual ResultCode      onUpdate(TaskManager& taskManager) = 0;

    void                    markInitialized() { m_initialized = true; } 
    //ResultCode initializeThreads();
    //ResultCode cleanUpThreads();



private:
    ResultCode              startProcesses();
    void                    stopProcesses();
    void                    startWorkerPool();
    void                    stopWorkerPool();
    void                    flushStopRequests();
    void                    flushNewRequests();

    TaskManager                             m_taskManager;
    MessageBus*                             m_pMessageBusRef;
    Engine::Scene*                          m_pScene;
    std::map<ProcessId, TaskProcess>        m_taskProcesses;
    std::map<Hash64, MessageBus*>           m_messageBusMap;
    Bool                                    m_initialized;
    Bool                                    m_isRunning;
    ThreadPool                              m_workerPool;
    std::string                             m_appName;

    std::vector<ProcessId>                                                      m_stopRequests;
    std::vector<std::tuple<TaskProcess::OnProcessTask, const char*, ProcessId>> m_newRequests;
};


// Main loop to run your application. Be sure to call this on the main thread!
// Allows for configuration of job tasks to the engine, along with other configs
// regarding your game.
namespace MainThreadLoop {


RecluseEngine_PUBLIC_API ResultCode        loadApp(Application* pApp);
RecluseEngine_PUBLIC_API ResultCode        run();
RecluseEngine_PUBLIC_API ResultCode        initialize();
RecluseEngine_PUBLIC_API ResultCode        cleanUp();
RecluseEngine_PUBLIC_API void           setFixedTickRate(F32 tickRateSeconds);

RecluseEngine_PUBLIC_API Application*   getApp();
RecluseEngine_PUBLIC_API F32            getFixedTickRate();

// This is operating system specific.
RecluseEngine_PUBLIC_API Bool           isMainThread();
RecluseEngine_PUBLIC_API MessageBus*    getMessageBus();
} // MainThreadLoop
} // Recluse