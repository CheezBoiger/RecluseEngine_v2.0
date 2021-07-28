//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Memory/StackAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {
namespace Engine {


void Scene::addGameObject(GameObject* obj)
{
    if (!obj) return;
    
    auto iter = std::find(m_gameObjects.begin(), m_gameObjects.end(), obj);
    if (iter != m_gameObjects.end()) {
    
        R_WARN("Scene", "Game object already exists in scene! Ignoring %s", __FUNCTION__);

        return;
    }
    
    // Add in the object, otherwise.
    m_gameObjects.push_back(obj);
}


void Scene::update(const RealtimeTick& tick)
{
    Allocation output   = { };
    ErrType err         = REC_RESULT_OK;

    err = m_gameMemAllocator->allocate(&output, 8ull * m_gameObjects.size(), ARCH_PTR_SZ_BYTES);

    PtrType currObjectPtr   = m_gameMemAllocator->getBaseAddr();
    PtrType top             = m_gameMemAllocator->getTop();
    memcpy((void*)output.ptr, m_gameObjects.data(), m_gameObjects.size() * 8ull);
    
    while (currObjectPtr < top) {
    
        GameObject* pObject                 = *(GameObject**)currObjectPtr;
        std::vector<GameObject*>& children  = pObject->getChildren();

        if (!children.empty()) {

            // We need to allocate some more.
            err = m_gameMemAllocator->allocate(&output, 8llu * children.size(), ARCH_PTR_SZ_BYTES);
            memcpy((void*)output.ptr, children.data(), children.size() * 8ull);
            // Update the top.
            top = m_gameMemAllocator->getTop();

        }

        // Update the game logic.
        pObject->update(tick);
        
        currObjectPtr += R_ALLOC_MASK(8ull, ARCH_PTR_SZ_BYTES);
    }

    // Reset the allocator when done...
    m_gameMemAllocator->reset();
}


void Scene::initialize()
{
    U64 szBytes = (1<<12) * ARCH_PTR_SZ_BYTES;    
    m_gameMemPool = new MemoryPool(szBytes);
    m_gameMemAllocator = new StackAllocator();
    m_gameMemAllocator->initialize(m_gameMemPool->getBaseAddress(), m_gameMemPool->getTotalSizeBytes());
}


void Scene::destroy()
{
    m_gameMemAllocator->cleanUp();
    delete m_gameMemAllocator;
    m_gameMemAllocator = nullptr;

    delete m_gameMemPool;
    m_gameMemPool = nullptr;
}


GameObject* Scene::getGameObject(U32 idx)
{
    return m_gameObjects[idx];
}


GameObject* Scene::findGameObject(const std::string& name)
{
    for (auto* gameObject : m_gameObjects) {

        if (gameObject->getName() == name) {

            return gameObject;

        }

    }

    return nullptr;
}


void Scene::removeGameObject(U32 idx)
{
    m_gameObjects.erase(m_gameObjects.begin() + idx);
}
} // Engine
} // Recluse