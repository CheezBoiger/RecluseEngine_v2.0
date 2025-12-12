//
#include "Recluse/Scene/Scene.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Renderer/Renderer.hpp"

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

ResultCode Scene::addEntity(const RGUID& entity, const RGUID& parent)
{
    if (!entity.isValid()) return RecluseResult_NullPtrExcept;
    
    auto iter = std::find(m_entities.begin(), m_entities.end(), entity);
    if (iter != m_entities.end()) 
    {
        R_WARN("Scene", "Game object already exists in scene! Ignoring %s", __FUNCTION__);
        return RecluseResult_AlreadyExists;
    }
    
    // Add in the object, otherwise.
    
    m_entities.push_back(entity);

    m_hierarchy.add(entity, parent);

    return RecluseResult_Ok;
}


void Scene::initialize()
{
}


void Scene::destroy()
{
}


RGUID Scene::getEntity(U32 idx)
{
    return m_entities[idx];
}


ECS::GameEntity* Scene::findEntity(const std::string& name)
{
    for (const auto& entityRGUID : m_entities) 
    {
        ECS::GameEntity* entity = ECS::GameEntity::findEntity(entityRGUID);

        if (entity->getName() == name) 
        {
            return entity;
        }
    }

    return nullptr;
}


ResultCode Scene::removeEntity(U32 idx)
{
    if (idx >= m_entities.size())
    {   
        R_ASSERT_FORMAT(idx >= m_entities.size(), "Attempting to access entity array with idx=%d, which is out of bounds! (size=%d)", idx, static_cast<U32>(m_entities.size()));
        return RecluseResult_OutOfBounds;
    }

    RGUID entity = m_entities[idx];
    ResultCode result = m_hierarchy.remove(entity);
    
    if (result == RecluseResult_Ok)
    {
    }

    m_entities.erase(m_entities.begin() + idx);
    return RecluseResult_Ok;
}


ResultCode Scene::save(Archive* pArchive)
{
    ResultCode result = RecluseResult_Ok;

    // Save the scene header first!
    {
        SceneHeaderInfo header  = { };
        header.numGameObjects   = m_entities.size();
        header.numLights        = 0ull;
        memcpy(header.sceneName, m_name.data(), 512);

        result = pArchive->write(&header, sizeof(SceneHeaderInfo));
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(m_name.c_str(), "Failed to write scene header to archive!");
        return result;
    }

    result = serialize(pArchive);

    return result;
}


ResultCode Scene::load(Archive* pArchive)
{
    ResultCode result = RecluseResult_Ok;

    SceneHeaderInfo header = { };
    
    result = pArchive->read(&header, sizeof(SceneHeaderInfo));

    if (result != RecluseResult_Ok) 
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


ResultCode Scene::serialize(Archive* pArchive) const
{
    return RecluseResult_NoImpl;
}


ResultCode Scene::deserialize(Archive* pArchive)
{
    return RecluseResult_NoImpl;
}


void Scene::drawDebug(ECS::Registry* registry, DebugRenderer* debugRenderer)
{
}
} // Engine
} // Recluse