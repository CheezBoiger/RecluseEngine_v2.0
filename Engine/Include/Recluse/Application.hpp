//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Scene/SceneLoader.hpp"

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Threading/ThreadPool.hpp"

namespace Recluse {


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

    virtual ~Application() { }    

    virtual void update(const RealtimeTick& tick) { }

    virtual void cleanUp() { }
    virtual void init() { }

private:
    
};


// Main loop to run your application. Be sure to call this on the main thread!
// Allows for configuration of job tasks to the engine, along with other configs
// regarding your game.
class R_PUBLIC_API MainThreadLoop {
public:
    static ErrType launchApp(const Application* pApp);

    static ErrType run();

    static ErrType loadJobThreads(SizeT jobId, JobTypeFlags flags);

    static Application* getApp();
private:
    static Application* k_pApp;
    static ThreadPool* k_pThreadPool;
};
} // Recluse