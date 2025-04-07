//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Serialization/Serializable.hpp"


#include "RecluseEngine_exports.hpp"

#include <vector>

namespace Recluse {

class MemoryPool;
class Allocator;
class MessageBus;

namespace Engine {

class Camera;
class DebugRenderer;

// Scene holds the hierarchical structure of the world. Can and should be used to 
// determine the order of entities in the world, by which who is their parent, and children,
// the lights and cameras in the world, and the probes that make up the overall environment.
//
class RecluseEngine_PUBLIC_API Scene : public Serializable 
{
public:
    Scene(const std::string& name = std::string()) : m_name (name) { }

    virtual ~Scene() { }

    void initialize();
    void destroy();

    ResultCode                     addEntity(ECS::GameEntity* pGameObject);
    ResultCode                     removeEntity(U32 idx);
    ResultCode                     removeEntity(const RGUID& guid);
    ECS::GameEntity*               findEntity(const std::string& name);
    ECS::GameEntity*               findEntity(const RGUID& guid);
    ECS::GameEntity*               getEntity(U32 idx);

    void                           setName(const std::string& name);
    const std::string&             getName() const { return m_name; }

    // Get game objects inside this scene.
    //
    const std::vector<ECS::GameEntity*>& getEntities() const { return m_entities; }

    // Serialize the scene.
    ResultCode                     save(Archive* pArchive);
    
    // Deserialize the serialize.
    ResultCode                     load(Archive* pArchive);

    // add a camera to the scene.
    void                                        addCamera(Camera* camera) { m_cameras.emplace_back(camera); }

    // Get the main camera in the scene.
    Camera*                                     getMainCamera() const { return m_cameras[0]; }
    Camera*                                     getCamera(U32 index) { return m_cameras[index]; }

    void                                        drawDebug(ECS::Registry* registry, DebugRenderer* debugRenderer);

protected:

    // Serialize the given scene. This should be used for 
    // custom scenes.
    virtual ResultCode                serialize(Archive* pArchive) const override;

    // Deserialize the scene from the given archive.
    //
    virtual ResultCode                deserialize(Archive* pArchive) override;

    // Set up the scene. Usually should be called if the scene is new, and 
    // needs setting up.
    virtual ResultCode                setUp() { return RecluseResult_NoImpl; }

    // Teardown the scene, for when any objects initialized, should be cleaned up 
    // by the scene.
    virtual ResultCode                tearDown() { return RecluseResult_NoImpl; }

private:

    // Game objects in the scene.
    std::vector<ECS::GameEntity*>       m_entities;
    std::string                         m_name;

    // cameras set in scene.
    // the index 0 is always the main camera.
    std::vector<Camera*>                m_cameras;
};
} // Engine
} // Recluse