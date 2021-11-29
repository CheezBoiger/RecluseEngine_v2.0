//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Application.hpp"

namespace Recluse {


template<typename ModuleImpl>
class EngineModule {
public:
    static ModuleImpl* getMain() {
        static ModuleImpl k_pMain;
        return &k_pMain;
    }

    static const char* getModuleName() {
        return R_STRINGIFY(ModuleImpl);
    }
    
    virtual ~EngineModule() { }

    virtual ErrType onInitializeModule(Application* pApp) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType onCleanUpModule(Application* pApp) { return REC_RESULT_NOT_IMPLEMENTED; }
    
    ErrType initializeModule(Application* pApp) {
        isActive() = true;
        return onInitializeModule(pApp); 
    }

    ErrType cleanUpModule(Application* pApp) { 
        isActive() = false;
        return onCleanUpModule(pApp); 
    }

    Bool& isActive() {
        static Bool active = false;
        return active;
    }

    Bool isRunning() const { return m_isRunning; }

    void enableRunning(Bool enable) { m_isRunning = enable; }

private:
    Bool m_isRunning = false;
};
} // Recluse