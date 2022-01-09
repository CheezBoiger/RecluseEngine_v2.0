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
        : m_currentCamera(nullptr)
        , m_gameMemAllocator(nullptr)
        , m_gameMemPool(nullptr) { }

    virtual ~Scene() { }

    R_PUBLIC_API void initialize();
    R_PUBLIC_API void destroy();

    R_PUBLIC_API void addGameObject(GameObject* pGameObject);
    R_PUBLIC_API void removeGameObject(U32 idx);
    R_PUBLIC_API GameObject* findGameObject(const std::string& name);
    R_PUBLIC_API GameObject* getGameObject(U32 idx);

    R_PUBLIC_API void setName(const std::string& name);
    const std::string& getName() const { return m_name; }

    // Get game objects inside this scene.
    //
    R_PUBLIC_API std::vector<GameObject*>& getGameObjects() { return m_gameObjects; }

    // Serialize the scene.
    R_PUBLIC_API ErrType save(Archive* pArchive);
    
    // Deserialize the serialize.    
    R_PUBLIC_API ErrType load(Archive* pArchive);

    // Update the scene using the tick. Can be overwritten 
    virtual R_PUBLIC_API void update(const RealtimeTick& tick);

    // 
    void setCamera(Camera* camera) { m_currentCamera = camera; }

    Camera* getCurrentCamera() const { return m_currentCamera; }
protected:

    // Serialize the given scene. This should be used for 
    // custom scenes.
    virtual R_PUBLIC_API ErrType serialize(Archive* pArchive) override;

    // Deserialize the scene from the given archive.
    //
    virtual R_PUBLIC_API ErrType deserialize(Archive* pArchive) override;

    // Set up the scene. Usually should be called if the scene is new, and 
    // needs setting up.
    virtual R_PUBLIC_API ErrType setUp() { return REC_RESULT_NOT_IMPLEMENTED; }

    // Teardown the scene, for when any objects initialized, should be cleaned up 
    // by the scene.
    virtual R_PUBLIC_API ErrType tearDown() { return REC_RESULT_NOT_IMPLEMENTED; }

private:

    // Game objects in the scene.
    std::vector<GameObject*> m_gameObjects;
    std::string                 m_name;

    // Current camera set in scene.
    Camera*                     m_currentCamera;

    MemoryPool*                 m_gameMemPool;
    Allocator*                  m_gameMemAllocator;
};
} // Engine
} // Recluse