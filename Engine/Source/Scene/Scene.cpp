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

void Scene::addEntity(ECS::GameEntity* obj)
{
    if (!obj) return;
    
    auto iter = std::find(m_entities.begin(), m_entities.end(), obj);
    if (iter != m_entities.end()) 
    {
        R_WARN("Scene", "Game object already exists in scene! Ignoring %s", __FUNCTION__);
        return;
    }
    
    // Add in the object, otherwise.
    m_entities.push_back(obj);
}


void Scene::update(const RealtimeTick& tick)
{
    for (auto& system : m_systems)
    {
        system->updateComponents(tick);
    }
}


void Scene::initialize()
{
}


void Scene::destroy()
{
}


ECS::GameEntity* Scene::getEntity(U32 idx)
{
    return m_entities[idx];
}


ECS::GameEntity* Scene::findEntity(const std::string& name)
{
    for (auto* entity : m_entities) 
    {
        if (entity->getName() == name) 
        {
            return entity;
        }
    }

    return nullptr;
}


void Scene::removeEntity(U32 idx)
{
    m_entities.erase(m_entities.begin() + idx);
}


ErrType Scene::save(Archive* pArchive)
{
    ErrType result = R_RESULT_OK;

    // Save the scene header first!
    {
        SceneHeaderInfo header  = { };
        header.numGameObjects   = m_entities.size();
        header.numLights        = 0ull;
        memcpy(header.sceneName, m_name.data(), 512);

        result = pArchive->write(&header, sizeof(SceneHeaderInfo));
    }

    if (result != R_RESULT_OK) 
    {
        R_ERR(m_name.c_str(), "Failed to write scene header to archive!");
        return result;
    }

    result = serialize(pArchive);    
}


ErrType Scene::load(Archive* pArchive)
{
    ErrType result = R_RESULT_OK;

    SceneHeaderInfo header = { };
    
    result = pArchive->read(&header, sizeof(SceneHeaderInfo));

    if (result != R_RESULT_OK) 
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
    return R_RESULT_NO_IMPL;
}


ErrType Scene::deserialize(Archive* pArchive)
{
    return R_RESULT_NO_IMPL;
}
} // Engine
} // Recluse