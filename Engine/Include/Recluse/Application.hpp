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
#include <map>
#include <list>
#include <functional>

#define R_BUILD_RETAIL                (0)
#define R_BUILD_RELEASE               (0)
#define R_BUILD_DEBUG                 (0)
#define R_BUILD_DEVELOPER             (0)

#define R_CLIENT                (1 << 0)
#define R_SERVER                (1 << 1)
#define R_CLIENT_AND_SERVER     (R_CLIENT | R_SERVER)

#define R_NET_TYPE              (R_CLIENT)

namespace Recluse {

class Window;
class Application;

enum JobType 
{
    JOB_TYPE_MAIN = 0,      //< Main will always be 0
    JOB_TYPE_SIMULATION,
    JOB_TYPE_RENDERER,
    JOB_TYPE_PHYSICS,
    JOB_TYPE_AUDIO,
    JOB_TYPE_ANIMATION,
    JOB_TYPE_AI,
    JOB_TYPE_NETWORK
};


typedef U32 JobTypeFlags;

// Application interface for your application.
// This should, and would be integrated into your game, in order to 
// connect to the engine components, as well as the editor system.
class R_PUBLIC_API Application 
{
public:

    Application()
        : m_pWindowRef(nullptr)
        , m_pScene(nullptr)
        , m_pMessageBusRef(nullptr)
        , m_initialized(false)
    { }

    virtual         ~Application() { }    

    // System update.
    virtual void    update(const RealtimeTick& tick);

    ErrType cleanUp() 
    { 
        ErrType result = onCleanUp();
        if (result == RecluseResult_Ok)
        {
            // Do no destroy window, this should be handled externally.
            m_pWindowRef = nullptr;
            m_initialized = false;
        }
        return result;
    }

    ErrType init(Window* pWindowHandle, MessageBus* pMessageBus) 
    { 
        m_pWindowRef        = pWindowHandle;
        m_pMessageBusRef    = pMessageBus;
        ErrType result = onInit();
        if (result == RecluseResult_Ok)
            markInitialized();
        return result;
    }

    ErrType         loadJobThread(JobTypeFlags flags, ThreadFunction func);
    Thread*         getJobThread(JobType jobType);

    Engine::Scene*  getScene() { return m_pScene; }
    Window*         getWindow() { return m_pWindowRef; }
    MessageBus*     getMessageBus() { return m_pMessageBusRef; }

    inline Bool isInitialized() const { return m_initialized; }

    // Registering systems can provide more priority for updating.
    Bool registerSystem(ECS::AbstractSystem* pSystem)
    {
        m_systems.push(pSystem);
        return true;
    }

protected:

    //! Application specific initialization. This requires 
    //! individual app owners to initialize each module for their 
    //! game system.
    virtual ErrType onInit() = 0;
    
    //! Like onInit(), cleans up all application defined resources.
    //! Requires all modules initialized, to be cleaned up manually as well.
    virtual ErrType onCleanUp() = 0;

    void markInitialized() { m_initialized = true; } 

private:
    Window*                     m_pWindowRef;
    MessageBus*                 m_pMessageBusRef;
    Engine::Scene*              m_pScene;
    std::list<Thread>           m_threads;
    std::map<JobType, Thread*>  m_jobThreads;

    // Priority system queue.
    std::priority_queue
        <
            ECS::AbstractSystem*,
            std::vector<ECS::AbstractSystem*>,
            ECS::SystemPointerComparer
        >                       m_systems;

    Bool                        m_initialized;
};


// Main loop to run your application. Be sure to call this on the main thread!
// Allows for configuration of job tasks to the engine, along with other configs
// regarding your game.
namespace MainThreadLoop {


R_PUBLIC_API ErrType        loadApp(Application* pApp);
R_PUBLIC_API ErrType        run();
R_PUBLIC_API ErrType        initialize();
R_PUBLIC_API ErrType        cleanUp();
R_PUBLIC_API void           setFixedTickRate(F32 tickRateSeconds);

R_PUBLIC_API Application*   getApp();
R_PUBLIC_API F32            getFixedTickRate();

// This is operating system specific.
R_PUBLIC_API Bool           isMainThread();
R_PUBLIC_API MessageBus*    getMessageBus();
} // MainThreadLoop
} // Recluse