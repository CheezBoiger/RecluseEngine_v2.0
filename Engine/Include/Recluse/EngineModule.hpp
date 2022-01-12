//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/Threading/Threading.hpp"

namespace Recluse {


template<typename ModuleImpl>
class EngineModule 
{
public:
    static ModuleImpl* getMain() 
    {
        static ModuleImpl k_pMain;
        return &k_pMain;
    }

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
        return getMain()->cleanUpInstance(pApp);
    }

private:
    virtual ErrType onInitializeModule(Application* pApp) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType onCleanUpModule(Application* pApp) { return REC_RESULT_NOT_IMPLEMENTED; }
    
    ErrType initializeInstance(Application* pApp) 
    {
        isActive() = true;
        m_sync = createMutex(R_STRINGIFY(ModuleImpl));
        return onInitializeModule(pApp); 
    }

    ErrType cleanUpInstance(Application* pApp) 
    { 
        ErrType result = onCleanUpModule(pApp);
        if (result == REC_RESULT_OK) 
        {
            isActive() = false;
            destroyMutex(m_sync);
        }

        return result; 
    }

public:

    Bool& isActive() 
    {
        static Bool active = false;
        return active;
    }

    Bool isRunning() const { return m_isRunning; }

    void enableRunning(Bool enable) { ScopedLock lck(m_sync); m_isRunning = enable; }

    Mutex getMutex() { return m_sync; }

private:
    Bool m_isRunning = false;
    Mutex m_sync;
};
} // Recluse