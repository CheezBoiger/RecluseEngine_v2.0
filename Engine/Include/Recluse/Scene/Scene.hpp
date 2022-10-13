//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include <vector>

namespace Recluse {

class MemoryPool;
class Allocator;

namespace Engine {

class Camera;

class Scene : public Serializable 
{
public:
    Scene(const std::string& name = std::string()) : m_name (name) { }

    virtual ~Scene() { }

    R_PUBLIC_API void initialize();
    R_PUBLIC_API void destroy();

    R_PUBLIC_API void addEntity(ECS::GameEntity* pGameObject);
    R_PUBLIC_API void removeEntity(U32 idx);
    R_PUBLIC_API ECS::GameEntity* findEntity(const std::string& name);
    R_PUBLIC_API ECS::GameEntity* getEntity(U32 idx);

    R_PUBLIC_API void setName(const std::string& name);
    const std::string& getName() const { return m_name; }

    // Get game objects inside this scene.
    //
    R_PUBLIC_API std::vector<ECS::GameEntity*>& getEntities() { return m_entities; }

    // Serialize the scene.
    R_PUBLIC_API ErrType save(Archive* pArchive);
    
    // Deserialize the serialize.    
    R_PUBLIC_API ErrType load(Archive* pArchive);

    // Update the scene systems. This can also be overridden to allow multithreading purposes.
    virtual R_PUBLIC_API void update(const RealtimeTick& tick);

    // add a camera to the scene.
    void addCamera(Camera* camera) { m_cameras.emplace_back(camera); }

    // Get the main camera in the scene.
    Camera* getMainCamera() const { return m_cameras[0]; }

    Camera* getCamera(U32 index) { return m_cameras[index]; }

    // Register a system into the scene.
    void registerSystem(ECS::AbstractSystem* pSystem) 
    {
        // Registering any system must Initialize first.
        pSystem->initialize(); 
        m_systems.push_back(pSystem); 
    }

protected:

    // Serialize the given scene. This should be used for 
    // custom scenes.
    virtual R_PUBLIC_API ErrType serialize(Archive* pArchive) override;

    // Deserialize the scene from the given archive.
    //
    virtual R_PUBLIC_API ErrType deserialize(Archive* pArchive) override;

    // Set up the scene. Usually should be called if the scene is new, and 
    // needs setting up.
    virtual R_PUBLIC_API ErrType setUp() { return R_RESULT_NO_IMPL; }

    // Teardown the scene, for when any objects initialized, should be cleaned up 
    // by the scene.
    virtual R_PUBLIC_API ErrType tearDown() { return R_RESULT_NO_IMPL; }

private:

    // Game objects in the scene.
    std::vector<ECS::GameEntity*>       m_entities;
    std::string                         m_name;

    // cameras set in scene.
    // the index 0 is always the main camera.
    std::vector<Camera*>                m_cameras;

    // Systems to update.
    std::vector<ECS::AbstractSystem*>   m_systems;
};
} // Engine
} // Recluse