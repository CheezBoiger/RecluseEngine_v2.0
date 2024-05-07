//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Threading/ThreadPool.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include <map>
#include <memory>
#include <functional>

namespace Recluse {
namespace Engine {

// This must be defined for every module that is used by the engine.
#define DEFINE_ENGINE_MODULE(ModuleImpl) \
    ModuleImpl* Recluse::Engine::EngineModule<ModuleImpl>::getMain() \
    { \
        static ModuleImpl k_main; \
        return &k_main; \
    }

typedef Hash64 EnginePluginId;


// TODO(Garcia): Need to use unique ptrs, but they are being so annoying right now.
//               Instead, using raw new/delete for plugins.
#define DEFINE_MODULE_PLUGIN(PluginImpl, ModuleImpl, PluginId, UsesLibrary, PluginLibraryName) \
    public: \
    static EnginePluginId obtainId() { return PluginId; } \
    static ModulePlugin<ModuleImpl>* create() \
    { \
         return new PluginImpl(); \
    } \
    static char* GetLibraryName() { return #PluginLibraryName; } \
    static bool IsLibrary() { return UsesLibrary; }

template<typename ModuleImpl>
class ModulePlugin
{
public:
    virtual ~ModulePlugin() { }
    ModulePlugin() { }

    virtual ResultCode initialize(ModuleImpl* impl) { return RecluseResult_NoImpl; }
    virtual ResultCode cleanUp(ModuleImpl* impl) { return RecluseResult_NoImpl; } 

};

//! EngineModule defines the singleton module used by the game engine.
//! This usually handles the normal intantiation and destruction of the 
//! module system, which should not be created more than once.
template<typename ModuleImpl>
class EngineModule 
{
public:
    typedef ModulePlugin<ModuleImpl> Plugin;

    //! Get the main singleton of this engine module.
    static ModuleImpl* getMain();

    static const char* getModuleName() 
    {
        return R_STRINGIFY(ModuleImpl);
    }
    
    virtual ~EngineModule() { }

    static ResultCode initializeModule(Application* pApp)
    {
        return getMain()->initializeInstance(pApp);
    }

    static ResultCode cleanUpModule(Application* pApp)
    {
        ResultCode result = getMain()->cleanUpInstance(pApp);
        return result;
    }

protected:
    EngineModule() { }

private:

    //! On initialize.
    virtual ResultCode onInitializeModule(Application* pApp) { return RecluseResult_NoImpl; }
    //! On clean up.
    virtual ResultCode onCleanUpModule(Application* pApp) { return RecluseResult_NoImpl; }
    
    //! Member function that is used to begin instantiating the object.
    ResultCode initializeInstance(Application* pApp) 
    {
        m_sync = createMutex(R_STRINGIFY(ModuleImpl));
        m_isActive = true;
        return onInitializeModule(pApp); 
    }

    ResultCode cleanUpInstance(Application* pApp) 
    { 
        ResultCode result = onCleanUpModule(pApp);
        if (result == RecluseResult_Ok) 
        {
            m_isActive = false;
            destroyMutex(m_sync);
        }

        return result; 
    }

public:
    typedef std::function<ModulePlugin<ModuleImpl>*(EngineModule<ModuleImpl>*)> PluginCreationFunction;
    // Check if the engine module is active.
    Bool isActive() const 
    {
        return m_isActive;
    }

    Bool            isRunning() const { return m_isRunning; }
    void            enableRunning(Bool enable) { ScopedLock lck(m_sync); m_isRunning = enable; }
    Mutex           getMutex() { return m_sync; }

    ModulePlugin<ModuleImpl>* getPlugin(EnginePluginId id)
    {
        auto it = m_plugins.find(id);
        if (it == m_plugins.end())
            return nullptr;
        else
            return it->second;
    }

    template<typename Plugin>
    ResultCode addPlugin()
    {
        auto it = m_plugins.find(Plugin::obtainId());
        if (it == m_plugins.end())
        {
            // Don't initialize here, only initialize where the module itself can.
            ModulePlugin<ModuleImpl>* plugin = Plugin::create();
            m_plugins.insert(std::make_pair(Plugin::obtainId(), std::move(plugin)));
            return RecluseResult_Ok;
        }
        return RecluseResult_AlreadyExists;
    }

    ResultCode cleanUpPlugins()
    {
        for (auto plugin : m_plugins)
        {
            plugin.second->cleanUp(nullptr);
            delete plugin.second;
        }
        m_plugins.clear();
        return RecluseResult_Ok;
    }

private:
    volatile Bool   m_isRunning = false;
    volatile Bool   m_isActive  = false;
    Mutex           m_sync;

    // Thread pool which we can use to launch how many threads.
    ThreadPool      m_threadPool;
    std::map<EnginePluginId, ModulePlugin<ModuleImpl>*> m_plugins;
};
} // Engine
} // Recluse