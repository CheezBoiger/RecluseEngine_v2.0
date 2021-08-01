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

class Scene : public Serializable {
public:
    Scene() 
        : m_currentCamera(nullptr)
        , m_gameMemAllocator(nullptr)
        , m_gameMemPool(nullptr) { }

    virtual ~Scene() { }

    R_EXPORT void initialize();
    R_EXPORT void destroy();

    R_EXPORT void addGameObject(GameObject* pGameObject);
    R_EXPORT void removeGameObject(U32 idx);
    R_EXPORT GameObject* findGameObject(const std::string& name);
    R_EXPORT GameObject* getGameObject(U32 idx);

    R_EXPORT void setName(const std::string& name);
    const std::string& getName() const { return m_name; }

    // Get game objects inside this scene.
    //
    R_EXPORT std::vector<GameObject*>& getGameObjects() { return m_gameObjects; }

    // Serialize the scene.
    R_EXPORT ErrType save(Archive* pArchive);
    
    // Deserialize the serialize.    
    R_EXPORT ErrType load(Archive* pArchive);

    // Update the scene using the tick. Can be overwritten 
    virtual R_EXPORT void update(const RealtimeTick& tick);

    // 
    void setCamera(Camera* camera) { m_currentCamera = camera; }

    Camera* getCurrentCamera() const { return m_currentCamera; }
protected:

    // Serialize the given scene. This should be used for 
    // custom scenes.
    virtual R_EXPORT ErrType serialize(Archive* pArchive) override;

    // Deserialize the scene from the given archive.
    //
    virtual R_EXPORT ErrType deserialize(Archive* pArchive) override;

    // Set up the scene. Usually should be called if the scene is new, and 
    // needs setting up.
    virtual R_EXPORT ErrType setUp() { return REC_RESULT_NOT_IMPLEMENTED; }

    // Teardown the scene, for when any objects initialized, should be cleaned up 
    // by the scene.
    virtual R_EXPORT ErrType tearDown() { return REC_RESULT_NOT_IMPLEMENTED; }

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