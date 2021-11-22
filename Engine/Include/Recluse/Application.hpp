//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Scene/SceneLoader.hpp"

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Threading/ThreadPool.hpp"

#include "Recluse/MessageBus.hpp"
#include <map>
#include <list>
#include <functional>

namespace Recluse {

class Window;
class Application;

enum JobType {
    JOB_TYPE_RENDERER,
    JOB_TYPE_PHYSICS,
    JOB_TYPE_AUDIO,
    JOB_TYPE_ANIMATION,
    JOB_TYPE_AI,
    JOB_TYPE_SIMULATION
};


typedef U32 JobTypeFlags;

class R_PUBLIC_API Application {
public:

    virtual         ~Application() { }    
    virtual void    update(const RealtimeTick& tick) { }

    ErrType cleanUp() { 
        m_initialized = false;
        return onCleanUp();
    }

    ErrType init() { 
        ErrType result = onInit();
        if (result == REC_RESULT_OK)
            markInitialized();
        return result;
    }

    ErrType         loadJobThread(JobTypeFlags flags, ThreadFunction func);
    Thread*         getJobThread(JobType jobType);

    Engine::Scene* getScene() { return m_pScene; }

    Bool isInitialized() const { return m_initialized; }

protected:

    virtual ErrType onInit() { return REC_RESULT_OK; }
    virtual ErrType onCleanUp() { return REC_RESULT_OK; }

    void markInitialized() { m_initialized = true; } 

private:
    Engine::Scene* m_pScene;
    std::list<Thread> m_threads;
    std::map<JobType, Thread*> m_jobThreads;
    Bool m_initialized;
};


// Main loop to run your application. Be sure to call this on the main thread!
// Allows for configuration of job tasks to the engine, along with other configs
// regarding your game.
class R_PUBLIC_API MainThreadLoop {
public:
    static ErrType      loadApp(Application* pApp);
    static ErrType      run();
    static ErrType      initialize();
    static ErrType      cleanUp();

    static Application* getApp();

    // This is operating system specific.
    static Bool isMainThread();
    static MessageBus* getMessageBus();

private:
    static Window*              k_pWindow;
    static Application*         k_pApp;
    static ThreadPool*          k_pThreadPool;
    static MessageBus*          k_pMessageBus;

};
} // Recluse