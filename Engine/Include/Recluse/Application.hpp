//
#pragma once
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

struct JobMessage : public AMessage {
    JobMessage(const std::string& msg) 
        : m_msg(msg) 
    {
        
    }

    virtual std::string getEvent() override {
        return m_msg;    
    }
private:
    std::string m_msg;
};

class R_PUBLIC_API Application {
public:

    Application()
        : m_pWindow(nullptr)
        , m_pScene(nullptr)
        , m_initialized(false)
    { }

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
    Window* getWindow() { return m_pWindow; }

    Bool isInitialized() const { return m_initialized; }

protected:

    virtual ErrType onInit() { return REC_RESULT_OK; }
    virtual ErrType onCleanUp() { return REC_RESULT_OK; }

    void markInitialized() { m_initialized = true; } 

private:
    Window* m_pWindow;
    Engine::Scene* m_pScene;
    std::list<Thread> m_threads;
    std::map<JobType, Thread*> m_jobThreads;
    Bool m_initialized;
};


// Main loop to run your application. Be sure to call this on the main thread!
// Allows for configuration of job tasks to the engine, along with other configs
// regarding your game.
namespace MainThreadLoop {


R_PUBLIC_API ErrType        loadApp(Application* pApp);
R_PUBLIC_API ErrType        run();
R_PUBLIC_API ErrType        initialize();
R_PUBLIC_API ErrType        cleanUp();

R_PUBLIC_API Application*   getApp();

// This is operating system specific.
R_PUBLIC_API Bool           isMainThread();
R_PUBLIC_API MessageBus*    getMessageBus();
} // MainThreadLoop
} // Recluse