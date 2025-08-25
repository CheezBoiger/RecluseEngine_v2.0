//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Application.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Threading/ThreadPool.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include <map>
#include <list>
#include <vector>
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
    static Hash64 GetNameHash() const { return recluseHashFast(GetLibraryName(), sizeof(GetLibraryName()); } \
    static bool IsLibrary() { return UsesLibrary; }

class ModulePluginHandler
{
public:
    static void destroy(ModulePluginHandler* handler)
    {
        delete handler;
    }
};


// Plugin module interface. Used for assigning plugins to an engine module.
template<typename ModuleImpl, Bool UniqueOnly = true>
class ModulePlugin : public ModulePluginHandler
{
public:
    virtual ~ModulePlugin() { }
    ModulePlugin() { }

    virtual ResultCode initialize(ModuleImpl* impl) { return RecluseResult_NoImpl; }
    virtual ResultCode cleanUp(ModuleImpl* impl) { return RecluseResult_NoImpl; } 

    virtual ResultCode preSetup(ModuleImpl* impl) { return RecluseResult_NoImpl; }
    virtual ResultCode postSetup(ModuleImpl* impl) { return RecluseResult_NoImpl; }

    // Checks if there can only be one unique plugin with this PluginId.
    static Bool isUnique() { return UniqueOnly; }
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
        getMain()->cleanUpMessageBuses();
        return result;
    }
    
    // Link a message bus to this module. The module will listen on any events
    // that are fired to the bus.
    ResultCode linkMessageBus(MessageBus* bus)
    {
        auto it = m_messageBusMap.find(bus->getId());
        ResultCode result = RecluseResult_AlreadyExists;
        if (it == m_messageBusMap.end())
        {
            bus->addReceiver(getModuleName(), [&] (const EventMessage& message) -> ResultCode
                { 
                    return onEvent(message);
                });
            m_messageBusMap.insert(std::make_pair(bus->getId(), bus));
            result = RecluseResult_Ok;
        }
        return result;
    }

    // Unlink a message bus from this module. Module will no longer hear 
    // any events from the unlinked bus.
    ResultCode unlinkMessageBus(MessageBus::Id busId)
    {
        ResultCode result = RecluseResult_NotFound;
        auto it = m_messageBusMap.find(busId);
        if (it != m_messageBusMap.end())
        {
            m_messageBusMap.erase(it);
            result = RecluseResult_Ok;
        }
        return result;
    }

    virtual ResultCode onEvent(const EventMessage& eventMessage) { return RecluseResult_NoImpl; }

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

    template<typename PluginClass = ModulePlugin<ModuleImpl>>
    PluginClass* getPlugin(EnginePluginId id, uint index = 0)
    {
        auto it = m_plugins.find(id);
        if (it == m_plugins.end())
            return nullptr;
        else
        {
            return it->second.empty() ? nullptr :  dynamic_cast<PluginClass*>(it->second[index]);
        }
    }

    uint getPluginCount(EnginePluginId id)
    {
        auto it = m_plugins.find(id);
        if (it == m_plugins.end())
            return 0;
        else
        {
            return it->second.size();
        }
    }

    template<typename Plugin>
    ResultCode addPlugin()
    {
        ResultCode result = RecluseResult_Ok;
        auto it = m_plugins.find(Plugin::obtainId());
        if (it == m_plugins.end())
        {
            // Don't initialize here, only initialize where the module itself can.
            ModulePlugin<ModuleImpl>* plugin = Plugin::create();
            //result = plugin->initialize(getMain());
            if (result == RecluseResult_Ok)
            {
                m_plugins[Plugin::obtainId].push_back(std::move(plugin));
            }
        }
        else
        {
            // some plugins exist for this. Make sure we aren't using the same one.
            if (Plugin::isUnique())
                result = RecluseResult_AlreadyExists;
            else
            {
                ModulePlugin<ModuleImpl>* plugin = Plugin::create();
                //result = plugin->initialize(getMain());
                if (result == RecluseResult_Ok)
                    it->second.push_back(std::move(plugin));
            }
        }
        return result;
    }

    ResultCode cleanUpPlugins()
    {
        for (auto& pluginList : m_plugins)
        {
            for (auto& plugin : pluginList.second)
            {
                plugin->cleanUp(getMain());
                ModulePluginHandler::destroy(plugin);
            }
            pluginList.second.clear();
        }
        m_plugins.clear();
        return RecluseResult_Ok;
    }

    ResultCode cleanUpMessageBuses()
    {
        m_messageBusMap.clear();
        return RecluseResult_Ok;
    }

private:
    volatile Bool   m_isRunning = false;
    volatile Bool   m_isActive  = false;
    Mutex           m_sync;

    // Thread pool which we can use to launch how many threads.
    ThreadPool      m_threadPool;
    std::map<EnginePluginId, std::vector<ModulePlugin<ModuleImpl>*>> m_plugins;

    // Message bus map.
    std::map<MessageBus::Id, MessageBus*> m_messageBusMap;
};
} // Engine
} // Recluse