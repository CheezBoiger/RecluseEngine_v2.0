//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {
namespace Engine {

struct SceneHeaderInfo 
{
    char    sceneName[512];
    U64     numGameObjects;
    U64     numLights;
};

void Scene::addGameObject(ECS::GameObject* obj)
{
    if (!obj) return;
    
    auto iter = std::find(m_gameObjects.begin(), m_gameObjects.end(), obj);
    if (iter != m_gameObjects.end()) 
    {
        R_WARN("Scene", "Game object already exists in scene! Ignoring %s", __FUNCTION__);
        return;
    }
    
    // Add in the object, otherwise.
    m_gameObjects.push_back(obj);
}


void Scene::update(const RealtimeTick& tick)
{
    Allocation output           = { };
    ErrType err                 = REC_RESULT_OK;
    LinearAllocator* allocator   = static_cast<LinearAllocator*>(m_gameMemAllocator);

    // First allocation is the total number of objects in the root view.
    // Resize wwhen required.
    err = allocator->allocate(&output, 8ull * m_gameObjects.size(), ARCH_PTR_SZ_BYTES);

    PtrType currObjectPtr   = allocator->getBaseAddr();
    PtrType top             = allocator->getTop();
    memcpy((void*)output.baseAddress, m_gameObjects.data(), m_gameObjects.size() * 8ull);
    
    // Perform a breadth first look up for all of our game objects to update..
    while (currObjectPtr < top) 
    {
        ECS::GameObject* pObject                    = *(ECS::GameObject**)currObjectPtr;
        std::vector<ECS::GameObject*>& children     = pObject->getChildren();

        if (!children.empty()) 
        {
            // We need to allocate some more.
            err = allocator->allocate(&output, 8llu * children.size(), ARCH_PTR_SZ_BYTES);
            memcpy((void*)output.baseAddress, children.data(), children.size() * 8ull);
            // Update the top.
            top = allocator->getTop();
        }

        // Update the game logic.
        pObject->update(tick);
        
        currObjectPtr += R_ALLOC_MASK(8ull, ARCH_PTR_SZ_BYTES);
    }

    // Reset the allocator when done...
    allocator->reset();
}


void Scene::initialize()
{
    U64 szBytes = (1<<12) * ARCH_PTR_SZ_BYTES;    
    m_gameMemPool = new MemoryPool(szBytes);
    m_gameMemAllocator = new LinearAllocator();
    m_gameMemAllocator->initialize(m_gameMemPool->getBaseAddress(), m_gameMemPool->getTotalSizeBytes());
}


void Scene::destroy()
{
    if (m_gameMemAllocator) 
    {
        m_gameMemAllocator->cleanUp();
        delete m_gameMemAllocator;
        m_gameMemAllocator = nullptr;
    }

    if (m_gameMemPool) 
    {
        delete m_gameMemPool;
        m_gameMemPool = nullptr;
    }
}


ECS::GameObject* Scene::getGameObject(U32 idx)
{
    return m_gameObjects[idx];
}


ECS::GameObject* Scene::findGameObject(const std::string& name)
{
    for (auto* gameObject : m_gameObjects) 
    {
        if (gameObject->getName() == name) 
        {
            return gameObject;
        }
    }

    return nullptr;
}


void Scene::removeGameObject(U32 idx)
{
    m_gameObjects.erase(m_gameObjects.begin() + idx);
}


ErrType Scene::save(Archive* pArchive)
{
    ErrType result = REC_RESULT_OK;

    // Save the scene header first!
    {
        SceneHeaderInfo header  = { };
        header.numGameObjects   = m_gameObjects.size();
        header.numLights        = 0ull;
        memcpy(header.sceneName, m_name.data(), 512);

        result = pArchive->write(&header, sizeof(SceneHeaderInfo));
    }

    if (result != REC_RESULT_OK) 
    {
        R_ERR(m_name.c_str(), "Failed to write scene header to archive!");
        return result;
    }

    result = serialize(pArchive);    
}


ErrType Scene::load(Archive* pArchive)
{
    ErrType result = REC_RESULT_OK;

    SceneHeaderInfo header = { };
    
    result = pArchive->read(&header, sizeof(SceneHeaderInfo));

    if (result != REC_RESULT_OK) 
    {
        return result;
    }

    result = deserialize(pArchive);

    m_name = header.sceneName;
    
    return result;
}


void Scene::setName(const std::string& name)
{
    m_name = name;
}


ErrType Scene::serialize(Archive* pArchive) 
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType Scene::deserialize(Archive* pArchive)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Engine
} // Recluse