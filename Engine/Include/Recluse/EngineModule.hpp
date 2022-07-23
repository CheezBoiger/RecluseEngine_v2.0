//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Threading/ThreadPool.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"


namespace Recluse {
namespace Engine {

// This must be defined for every module that is used by the engine.
#define DEFINE_ENGINE_MODULE(ModuleImpl) \
    ModuleImpl* Recluse::Engine::EngineModule<ModuleImpl>::getMain() \
    { \
        static ModuleImpl k_main; \
        return &k_main; \
    }

//! EngineModule defines the singleton module used by the game engine.
//! This usually handles the normal intantiation and destruction of the 
//! module system, which should not be created more than once.
template<typename ModuleImpl>
class EngineModule 
{
public:

    //! Get the main singleton of this engine module.
    static ModuleImpl* getMain();

    static const char* getModuleName() 
    {
        return R_STRINGIFY(ModuleImpl);
    }
    
    virtual ~EngineModule() { }

    static ErrType initializeModule(Application* pApp)
    {
        return getMain()->initializeInstance(pApp);
    }

    static ErrType cleanUpModule(Application* pApp)
    {
        ErrType result = getMain()->cleanUpInstance(pApp);
        return result;
    }

protected:
    EngineModule() { }

private:

    //! On initialize.
    virtual ErrType onInitializeModule(Application* pApp) { return REC_RESULT_NOT_IMPLEMENTED; }
    //! On clean up.
    virtual ErrType onCleanUpModule(Application* pApp) { return REC_RESULT_NOT_IMPLEMENTED; }
    
    //! Member function that is used to begin instantiating the object.
    ErrType initializeInstance(Application* pApp) 
    {
        m_sync = createMutex(R_STRINGIFY(ModuleImpl));
        m_isActive = true;
        return onInitializeModule(pApp); 
    }

    ErrType cleanUpInstance(Application* pApp) 
    { 
        ErrType result = onCleanUpModule(pApp);
        if (result == REC_RESULT_OK) 
        {
            m_isActive = false;
            destroyMutex(m_sync);
        }

        return result; 
    }

public:
    // Check if the engine module is active.
    Bool isActive() const 
    {
        return m_isActive;
    }

    Bool            isRunning() const { return m_isRunning; }
    void            enableRunning(Bool enable) { ScopedLock lck(m_sync); m_isRunning = enable; }
    Mutex           getMutex() { return m_sync; }

private:
    volatile Bool   m_isRunning = false;
    volatile Bool   m_isActive  = false;
    Mutex           m_sync;

    // Thread pool which we can use to launch how many threads.
    ThreadPool      m_threadPool;
};
} // Engine
} // Recluse