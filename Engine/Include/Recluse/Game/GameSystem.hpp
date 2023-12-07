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

namespace Recluse {
class MessageBus;
namespace Engine {
class Scene;
} // Engine
} // Recluse

namespace Recluse {
namespace ECS {

// Declaration types.
typedef Hash64 GameUUID;

// Forward declare game object.
class GameEntity;

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


//! System is the high level provision that oversees all
//! game components to their respect.
//! Systems are what hold the game logic in the world scene, and what 
//! will perform work on components that only it will be allowed to see.
class R_PUBLIC_API System : public Serializable
{
public:

    virtual ~System() { }

    template<typename SpecializedSys>
    static ECS::System* allocate()
    {
        return new SpecializedSys();
    }

    static ResultCode free(System* psystem) 
    {
         if (psystem)
            delete psystem;
        return RecluseResult_Ok;
    }

    // Gets a component from entity.
    template<typename ComponentType>
    ComponentType* obtainComponent(const RGUID& id)
    {
        ECS::GameEntity* entity = ECS::GameEntity::findEntity(id);
        return entity->getComponent<ComponentType>(m_scene);
    }

    // Gets a tuple of components from an entity. Any components not found,
    // will return nullptr.
    template<typename... Args>
    std::tuple<Args*...> obtainTuple(const RGUID& id)
    {
        std::tuple<Args*...> args = { obtainComponent<Args>(id)... }; 
        
        return args;
    }

    void                                setPriority(U32 priority) { m_priority = priority; }
    U32                                 getPriority() const { return m_priority; }

    // This system is required to update all components when necessary.
    void                                update(const RealtimeTick& tick) { onUpdate(tick); }

    ResultCode                          initialize(MessageBus* bus = nullptr)
    {
        return onInitialize(bus);
    }

    ResultCode         cleanUp()
    {
        return onCleanUp();
    }

    // Serialize the system and its components.
    virtual ResultCode      serialize(Archive* archive) override { return RecluseResult_NoImpl; }

    // Deserialize the system and its components.
    virtual ResultCode      deserialize(Archive* archive) override { return RecluseResult_NoImpl; }

    virtual const char*     getName() const { return "AbstractSystem"; }
    
    Engine::Scene* getScene() const { return m_scene; }
    void            setScene(Engine::Scene* scene) { m_scene = scene; }
private:
    // Allows initializing the system before on intialize().
    virtual ResultCode      onInitialize(MessageBus* bus = nullptr) { return RecluseResult_NoImpl; }

    // Allows cleaning up the system before releasing.
    virtual ResultCode      onCleanUp()                     { return RecluseResult_NoImpl; }

    // Intended to clear all components from the game world.
    virtual void            onClearAll()                       { }

    // To update all components in the world.
    virtual void            onUpdate(const RealtimeTick& tick)  { }

    // Priority value of this abstract system. This will be used to determine the 
    // order of which this system will operate.
    U32                 m_priority;
    
    Engine::Scene*      m_scene;
};


class SystemComparer
{
public:
    Bool operator()(const System& lh, const System& rh) const 
    {
        return lh.getPriority() < rh.getPriority();
    }
};


class SystemPointerComparer
{
public:
    Bool operator()(const System* lh, const System* rh) const
    {
        return lh->getPriority() < rh->getPriority();
    }
};

typedef void* SystemPtr;

} // ECS
} // Recluse