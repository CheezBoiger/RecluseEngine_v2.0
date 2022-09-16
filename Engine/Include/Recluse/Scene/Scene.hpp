//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Game/GameObject.hpp"
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
    Scene() 
        : m_gameMemAllocator(nullptr)
        , m_gameMemPool(nullptr) { }

    virtual ~Scene() { }

    R_PUBLIC_API void initialize();
    R_PUBLIC_API void destroy();

    R_PUBLIC_API void addGameObject(ECS::GameObject* pGameObject);
    R_PUBLIC_API void removeGameObject(U32 idx);
    R_PUBLIC_API ECS::GameObject* findGameObject(const std::string& name);
    R_PUBLIC_API ECS::GameObject* getGameObject(U32 idx);

    R_PUBLIC_API void setName(const std::string& name);
    const std::string& getName() const { return m_name; }

    // Get game objects inside this scene.
    //
    R_PUBLIC_API std::vector<ECS::GameObject*>& getGameObjects() { return m_gameObjects; }

    // Serialize the scene.
    R_PUBLIC_API ErrType save(Archive* pArchive);
    
    // Deserialize the serialize.    
    R_PUBLIC_API ErrType load(Archive* pArchive);

    // Update the scene using the tick. Can be overwritten 
    virtual R_PUBLIC_API void update(const RealtimeTick& tick);

    // add a camera to the scene.
    void addCamera(Camera* camera) { m_cameras.emplace_back(camera); }

    // Get the main camera in the scene.
    Camera* getMainCamera() const { return m_cameras[0]; }

    Camera* getCamera(U32 index) { return m_cameras[index]; }

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
    std::vector<ECS::GameObject*>   m_gameObjects;
    std::string                     m_name;

    // cameras set in scene.
    // the index 0 is always the main camera.
    std::vector<Camera*>            m_cameras;

    MemoryPool*                     m_gameMemPool;
    Allocator*                      m_gameMemAllocator;
};
} // Engine
} // Recluse