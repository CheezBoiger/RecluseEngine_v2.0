// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/MessageBus.hpp"

#include "RecluseEngine_exports.hpp"

#include <tuple>
#include <vector>

namespace Recluse {
class MessageBus;
namespace Engine {
class Renderer;
class DebugRenderer;
class Scene;
} // Engine
} // Recluse

namespace Recluse {
namespace ECS {

// Declaration types.
typedef Hash64 GameUUID;

// Forward declare game object.
class GameEntity;
class Registry;


#define R_CLASS_PUBLIC_DEFINE public:

#define R_CLASS_GUID_DECLARE_IMPLEMENTATION(_class) \
    static Recluse::ECS::GameUUID classGUID() { return recluseHash(#_class, sizeof(#_class)); } \
    virtual Recluse::ECS::GameUUID getClassGUID() const override { return classGUID(); }

#define R_CLASS_NAME_DECLARE_IMPLEMENTATION(_class) \
    static const char* className() { return #_class; } \
    virtual const char* getClassName() const override { return className(); }

#define R_CLASS_SYSTEM_NAME_DECLARE_IMPLEMENTATION(_system) \
    static const char* systemName() { return #_system; } \
    virtual const char* getName() const override { return systemName(); }    

// Declare a game component.
#define R_PUBLIC_DECLARE_GAME_ECS(_class) \
    R_CLASS_PUBLIC_DEFINE \
    R_CLASS_GUID_DECLARE_IMPLEMENTATION(_class) \
    R_CLASS_NAME_DECLARE_IMPLEMENTATION(_class)


// Required declare for the game system to be used. 
// Use the constructor you feel is important.
// A Default destructor is required in order to do final cleanups at the end of an application's life.
#define R_DECLARE_GAME_SYSTEM(_system) \
    R_CLASS_PUBLIC_DEFINE \
    R_CLASS_SYSTEM_NAME_DECLARE_IMPLEMENTATION(_system)



class AbstractSystem : public Serializable
{
public:
    virtual ~AbstractSystem() { }

    template<typename SpecializedSys>
    static ECS::AbstractSystem* allocate()
    {
        ECS::AbstractSystem* system = new SpecializedSys();
        ResultCode result = system->initialize();
        if (result != RecluseResult_Ok)
        {
            AbstractSystem::free(system);
            system = nullptr;
        }
        return system;
    }

    static ResultCode free(AbstractSystem* psystem) 
    {
         if (psystem)
            delete psystem;
        return RecluseResult_Ok;
    }

    void                     setPriority(U32 priority) { m_priority = priority; }
    U32                      getPriority() const { return m_priority; }

    // This system is required to update all components when necessary. 
    // \param Registry
    // \param tick
    // \param scene (Optional) 
    void                                update(Registry* registry, const RealtimeTick& tick, Engine::Scene* scene = nullptr) { onUpdate(registry, tick); }

    // Initializes this system. Returns Ok if the system was properly initialized.
    ResultCode                          initialize()
    {
        return onInitialize();
    }

    // Links a message bus to this system. Returns Ok if the bus was found, and removed from listening to,
    // otherwise returns NotFound.
    ResultCode linkMessageBus(MessageBus* bus)
    {
        ResultCode result = RecluseResult_Failed;
        if (bus)
        {
            auto it = m_messageBusMap.find(bus->getId());
            if (it == m_messageBusMap.end())
            {
                bus->addReceiver(getName(), [&] (const EventMessage& event) -> ResultCode { return onEvent(event); });
                m_messageBusMap.insert(std::make_pair(bus->getId(), bus));
                result = RecluseResult_Ok;
            }
        }
        return result;
    }

    // Unlinks a message bus to this system. Returns Ok if the bus was found, and removed from listening to,
    // otherwise returns NotFound.
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

    ResultCode         cleanUp()
    {
        ResultCode result = onCleanUp();
        if (result == RecluseResult_Ok)
            result = cleanUpMessageBuses();
        return result;
    }

    void                    drawDebug(Registry* registry, Engine::DebugRenderer* renderer) { onDrawDebug(registry, renderer); }

    // Serialize the system and its components.
    virtual ResultCode      serialize(Archive* archive) const override { return RecluseResult_NoImpl; }

    // Deserialize the system and its components.
    virtual ResultCode      deserialize(Archive* archive) override { return RecluseResult_NoImpl; }
    virtual const char*     getName() const { return "System"; }
    virtual ResultCode      onEvent(const EventMessage& event) { return RecluseResult_NoImpl; }

protected:

    // Allows initializing the system on intialize().
    virtual ResultCode      onInitialize() { return RecluseResult_NoImpl; }

    // Allows post initialization after all initialize systems.
    virtual ResultCode      onPostInitialize() { return RecluseResult_NoImpl; }

    // Allows cleaning up the system before releasing.
    virtual ResultCode      onCleanUp()                     { return RecluseResult_NoImpl; }

    // Intended to clear all components from the game world.
    virtual void            onClearAll()                       { }

    // To update all components in the world. 
    // \param scene The scene instance that we are updating on.
    virtual void            onUpdate(Registry* registry, const RealtimeTick& tick, Engine::Scene* scene = nullptr) { }

    // Updates all component in the world after onUpdate() calls have been made.
    virtual void            onPostUpdate(Registry* registry, const RealtimeTick& tick) { }

    virtual void            onDrawDebug(Registry* registry, Engine::DebugRenderer* context) { }

    ResultCode              cleanUpMessageBuses()
    {
        m_messageBusMap.clear();
        return RecluseResult_Ok;
    }

private:
    // Priority value of this abstract system. This will be used to determine the 
    // order of which this system will operate.
    U32                                     m_priority;
    std::map<MessageBus::Id, MessageBus*>   m_messageBusMap;
};


//! System is the high level provision that oversees all
//! game components to their respect.
//! Systems are what hold the game logic in the world scene, and what 
//! will perform work on components that only it will be allowed to see.
template<typename TypeComponent>
class System : public AbstractSystem
{
public:

    virtual ~System() { }

    // Gets a component from entity.
    template<typename ComponentType>
    ComponentType* obtainComponent(Registry* registry, const RGUID& id)
    {
        ECS::GameEntity* entity = ECS::GameEntity::findEntity(id);
        return (entity ? registry->getComponent<ComponentType>(id) : nullptr);
    }

    // Returns a tuple of components from an entity. Any components not found,
    // will return nullptr for each component not found.
    //
    // To use, specify as: obtainTuple<MyComponent0, MyComponent1, etc...>(registry, id)
    //
    // Where MyComponent0, MyComponent1, etc... are the components you want to query for this system.
    template<typename... ComponentTypes>
    std::tuple<ComponentTypes*...> obtainTuple(Registry* registry, const RGUID& id)
    {
        std::tuple<ComponentTypes*...> args = { obtainComponent<ComponentTypes>(registry, id)... }; 
        return args;
    }

    // Helper function to obtain all components from the corresponding scene, which should contain the given 
    // registry.
    std::vector<TypeComponent*> obtainComponents(Registry* registry)
    {
        ECS::ComponentRegistry<TypeComponent>* componentRegistry = registry->getComponentRegistry<TypeComponent>();
        return componentRegistry->getAllComponents();
    }

    // Serialize the system and its components.
    virtual ResultCode      serialize(Archive* archive) const override { return RecluseResult_NoImpl; }

    // Deserialize the system and its components.
    virtual ResultCode      deserialize(Archive* archive) override { return RecluseResult_NoImpl; }

    virtual const char*     getName() const override { return "System"; }

    // On event callback to be used for System.
    virtual ResultCode      onEvent(const EventMessage& event) override { return RecluseResult_NoImpl; }

protected:
    // Allows initializing the system before on intialize().
    virtual ResultCode      onInitialize() override { return RecluseResult_NoImpl; }

    // Allows cleaning up the system before releasing.
    virtual ResultCode      onCleanUp() override                    { return RecluseResult_NoImpl; }

    // Intended to clear all components from the game world.
    virtual void            onClearAll() override                       { }

    // To update all components in the world. 
    // \param scene The scene instance that we are updating on.
    virtual void            onUpdate(Registry* registry, const RealtimeTick& tick, Engine::Scene* scene = nullptr) override { }

    // Updates all component in the world after onUpdate() calls have been made.
    virtual void            onPostUpdate(Registry* registry, const RealtimeTick& tick) override { }

    virtual void            onDrawDebug(Registry* registry, Engine::DebugRenderer* context) override { }
};


class SystemComparer
{
public:
    Bool operator()(const AbstractSystem& lh, const AbstractSystem& rh) const 
    {
        return lh.getPriority() < rh.getPriority();
    }
};


class SystemPointerComparer
{
public:
    Bool operator()(const AbstractSystem* lh, const AbstractSystem* rh) const
    {
        return lh->getPriority() < rh->getPriority();
    }
};

typedef void* SystemPtr;

} // ECS
} // Recluse