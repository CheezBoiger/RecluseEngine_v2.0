//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include <vector>

namespace Recluse {

class MemoryPool;
class Allocator;
class MessageBus;

namespace Engine {

class Camera;

class Scene : public Serializable 
{
public:
    Scene(const std::string& name = std::string()) : m_name (name) { }

    virtual ~Scene() { }

    R_PUBLIC_API void initialize();
    R_PUBLIC_API void destroy();

    R_PUBLIC_API ResultCode                     addEntity(ECS::GameEntity* pGameObject);
    R_PUBLIC_API ResultCode                     removeEntity(U32 idx);
    R_PUBLIC_API ResultCode                     removeEntity(const RGUID& guid);
    R_PUBLIC_API ECS::GameEntity*               findEntity(const std::string& name);
    R_PUBLIC_API ECS::GameEntity*               findEntity(const RGUID& guid);
    R_PUBLIC_API ECS::GameEntity*               getEntity(U32 idx);

    R_PUBLIC_API void                           setName(const std::string& name);
    const std::string&                          getName() const { return m_name; }

    // Get game objects inside this scene.
    //
    R_PUBLIC_API const std::vector<ECS::GameEntity*>& getEntities() const { return m_entities; }

    // Serialize the scene.
    R_PUBLIC_API ResultCode                     save(Archive* pArchive);
    
    // Deserialize the serialize.
    R_PUBLIC_API ResultCode                     load(Archive* pArchive);

    // Update the scene systems. This can also be overridden to allow multithreading purposes.
    virtual R_PUBLIC_API void                   update(const RealtimeTick& tick);

    // add a camera to the scene.
    void                                        addCamera(Camera* camera) { m_cameras.emplace_back(camera); }

    // Get the main camera in the scene.
    Camera*                                     getMainCamera() const { return m_cameras[0]; }
    Camera*                                     getCamera(U32 index) { return m_cameras[index]; }

    // Adds a system into this scene. Systems are usually global, but in our engine, they are 
    // only global to our respective scene. Therefore, it is essential that any new scenes must 
    // use any old systems, if we require transitions! 
    //
    //! bus - MessageBus for which the system will listen to.
    template<typename Sys>
    void                                        addSystem(MessageBus* bus = nullptr, U32 priority = 0u)
    {
        ECS::System* system = ECS::System::allocate<Sys>();
        registerSystem(system, bus);
    }

    //! Adds a registry for a component, for managing such resources.
    template<typename TypeRegistry>
    void addRegistry()
    {
        ECS::ComponentUUID uuid = TypeRegistry::componentGUID();
        auto it = m_registries.find(uuid);
        if (it == m_registries.end())
        {
            ECS::AbstractRegistry* registry = ECS::AbstractRegistry::allocate<TypeRegistry>();
            m_registries.insert(std::make_pair(uuid, registry));
        }
    }

    // Get the registry with the given component type.
    template<typename ComponentType>
    ECS::Registry<ComponentType>* getRegistry() const
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_registries.find(uuid);
        if (it != m_registries.end())
        {
            return static_cast<ECS::Registry<ComponentType>*>(it->second);
        }
        return nullptr;
    }

    template<typename ComponentType>
    ResultCode addComponentForEntity(const RGUID& entityId)
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_registries.find(uuid);
        if (it != m_registries.end())
        {
            ECS::Registry<ComponentType>* registry = static_cast<ECS::Registry<ComponentType>*>(it->second);
            registry->allocateComponent(entityId);
            return RecluseResult_Ok;
        }
        return RecluseResult_Failed;
    }

    template<typename ComponentType>
    ResultCode removeComponent(const RGUID& entityId)
    {
        ECS::ComponentUUID uuid = ComponentType::classGUID();
        auto it = m_registries.find(uuid);
        if (it != m_registries.end())
        {
            ECS::Registry<ComponentType>* registry = static_cast<ECS::Registry<ComponentType>*>(it->second);
            registry->freeComponent(entityId);
            return RecluseResult_Ok;
        }
        return RecluseResult_Failed;
    }

    template<typename ComponentType>
    ComponentType* getComponentFromEntity(const RGUID& guid) const
    {
        ECS::GameEntity* entity = ECS::GameEntity::findEntity(guid);
        if (entity)
        {
            return entity->getComponent<ComponentType>();
        }
        return nullptr;
    }

protected:

    // Serialize the given scene. This should be used for 
    // custom scenes.
    virtual R_PUBLIC_API ResultCode                serialize(Archive* pArchive) const override;

    // Deserialize the scene from the given archive.
    //
    virtual R_PUBLIC_API ResultCode                deserialize(Archive* pArchive) override;

    // Set up the scene. Usually should be called if the scene is new, and 
    // needs setting up.
    virtual R_PUBLIC_API ResultCode                setUp() { return RecluseResult_NoImpl; }

    // Teardown the scene, for when any objects initialized, should be cleaned up 
    // by the scene.
    virtual R_PUBLIC_API ResultCode                tearDown() { return RecluseResult_NoImpl; }

private:

    // Register a system into the scene.
    R_PUBLIC_API void registerSystem(ECS::System* pSystem, MessageBus* bus);
    R_PUBLIC_API void unregisterSystems();
    R_PUBLIC_API void clearRegistries();

    // Game objects in the scene.
    std::vector<ECS::GameEntity*>                          m_entities;
    std::string                                            m_name;

    // cameras set in scene.
    // the index 0 is always the main camera.
    std::vector<Camera*>                                   m_cameras;

    // Systems to update.
    std::vector<ECS::System*>                              m_systems;
    // Registries holding components that correspond to this scene.
    std::map<ECS::ComponentUUID, ECS::AbstractRegistry*>   m_registries;
};
} // Engine
} // Recluse