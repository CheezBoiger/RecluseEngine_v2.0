//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Game/GameObject.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include <vector>

namespace Recluse {

class MemoryPool;
class StackAllocator;

namespace Engine {

class Camera;

class Scene : public Serializable {
public:
    Scene() 
        : m_currentCamera(nullptr)
        , m_gameMemAllocator(nullptr)
        , m_gameMemPool(nullptr) { }

    R_EXPORT void initialize();
    R_EXPORT void destroy();

    R_EXPORT void addGameObject(GameObject* pGameObject);
    R_EXPORT void removeGameObject(U32 idx);
    R_EXPORT GameObject* findGameObject(const std::string& name);
    R_EXPORT GameObject* getGameObject(U32 idx);

    // Get game objects inside this scene.
    //
    R_EXPORT std::vector<GameObject*>& getGameObjects() { return m_gameObjects; }

    // Serialize the scene.
    R_EXPORT ErrType serialize(Archive* pArchive) override { return REC_RESULT_NOT_IMPLEMENTED; }
    
    // Deserialize the serialize.    
    R_EXPORT ErrType deserialize(Archive* pArchive) override { return REC_RESULT_NOT_IMPLEMENTED; }

    // Update the scene using the tick.
    R_EXPORT void update(const RealtimeTick& tick);

    // 
    void setCamera(Camera* camera) { m_currentCamera = camera; }

    Camera* getCurrentCamera() const { return m_currentCamera; }
private:
    // Game objects in the scene.
    std::vector<GameObject*> m_gameObjects;
    std::string                 m_name;

    // Current camera set in scene.
    Camera*                     m_currentCamera;

    MemoryPool*                 m_gameMemPool;
    StackAllocator*             m_gameMemAllocator;
};
} // Engine
} // Recluse