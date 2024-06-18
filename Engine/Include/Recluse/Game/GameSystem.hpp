// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Time.hpp"

#include <tuple>
#include <vector>

namespace Recluse {
class MessageBus;
namespace Engine {
class Renderer;
class DebugRenderer;
} // Engine
} // Recluse

namespace Recluse {
namespace ECS {

// Declaration types.
typedef Hash64 GameUUID;

// Forward declare game object.
class GameEntity;
class Registry;

#define R_PUBLIC_DECLARE_GAME_ECS(_class) \
    public: \
    static Recluse::ECS::GameUUID classGUID() { return recluseHash(#_class, sizeof(#_class)); } \
    static const char* className() { return #_class; } \
    virtual Recluse::ECS::GameUUID getClassGUID() const override { return classGUID(); } \
    virtual const char* getClassName() const override { return className(); }


// Required declare for the game system to be used. 
// Use the constructor you feel is important.
// A Default destructor is required in order to do final cleanups at the end of an application's life.
#define R_DECLARE_GAME_SYSTEM(_system) \
    public: \
    static const char* systemName() { return #_system; } \
    virtual const char* getName() const override { return systemName(); }


class R_PUBLIC_API AbstractSystem : public Serializable
{
public:
    virtual ~AbstractSystem() { }

    template<typename SpecializedSys>
    static ECS::AbstractSystem* allocate()
    {
        return new SpecializedSys();
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
    void                                update(Registry* registry, const RealtimeTick& tick) { onUpdate(registry, tick); }

    ResultCode                          initialize(MessageBus* bus = nullptr)
    {
        return onInitialize(bus);
    }

    ResultCode         cleanUp()
    {
        return onCleanUp();
    }

    void                    drawDebug(Registry* registry, Engine::DebugRenderer* renderer) { onDrawDebug(registry, renderer); }

    // Serialize the system and its components.
    virtual ResultCode      serialize(Archive* archive) const override { return RecluseResult_NoImpl; }

    // Deserialize the system and its components.
    virtual ResultCode      deserialize(Archive* archive) override { return RecluseResult_NoImpl; }
    virtual const char*     getName() const { return "System"; }

protected:

    // Allows initializing the system before on intialize().
    virtual ResultCode      onInitialize(MessageBus* bus = nullptr) { return RecluseResult_NoImpl; }

    // Allows cleaning up the system before releasing.
    virtual ResultCode      onCleanUp()                     { return RecluseResult_NoImpl; }

    // Intended to clear all components from the game world.
    virtual void            onClearAll()                       { }

    // To update all components in the world. 
    // \param scene The scene instance that we are updating on.
    virtual void            onUpdate(Registry* registry, const RealtimeTick& tick) { }

    // Updates all component in the world after onUpdate() calls have been made.
    virtual void            onPostUpdate(Registry* registry, const RealtimeTick& tick) { }

    virtual void            onDrawDebug(Registry* registry, Engine::DebugRenderer* context) { }

private:
    // Priority value of this abstract system. This will be used to determine the 
    // order of which this system will operate.
    U32                 m_priority;
};


//! System is the high level provision that oversees all
//! game components to their respect.
//! Systems are what hold the game logic in the world scene, and what 
//! will perform work on components that only it will be allowed to see.
template<typename TypeComponent>
class R_PUBLIC_API System : public AbstractSystem
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
    template<typename... Args>
    std::tuple<Args*...> obtainTuple(Registry* registry, const RGUID& id)
    {
        std::tuple<Args*...> args = { obtainComponent<Args>(registry, id)... }; 
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

protected:
    // Allows initializing the system before on intialize().
    virtual ResultCode      onInitialize(MessageBus* bus = nullptr) override { return RecluseResult_NoImpl; }

    // Allows cleaning up the system before releasing.
    virtual ResultCode      onCleanUp() override                    { return RecluseResult_NoImpl; }

    // Intended to clear all components from the game world.
    virtual void            onClearAll() override                       { }

    // To update all components in the world. 
    // \param scene The scene instance that we are updating on.
    virtual void            onUpdate(Registry* registry, const RealtimeTick& tick) override { }

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