//
#include "Recluse/Renderer/RendererResources.hpp"
#include "Recluse/Messaging.hpp"

#include <unordered_map>
#include <unordered_set>

namespace Recluse {
namespace Engine {
namespace RenderDB {

std::unordered_map<RenderID, Texture2D*> k_textureMap;
std::unordered_map<RenderID, GPUBuffer*> k_gpuBufferMap;

Texture2D* getTexture2D(RenderID id)
{
    if (!isCachedTexture2D(id)) 
    {
        R_WARN("RenderDB", "Unable to find texture id=%d in map.", id);
        return nullptr;
    }

    return k_textureMap[id];
}


GPUBuffer* getGPUBuffer(RenderID id)
{
    if (!isCachedGPUBuffer(id)) 
    {
        R_WARN("RenderDB", "Unable to find gpu buffer id=%d", id);
        return nullptr;
    }

    return k_gpuBufferMap[id];
}


ErrType cacheGPUBuffer(RenderID id, GPUBuffer* pBuffer)
{
    if (isCachedGPUBuffer(id) && k_gpuBufferMap[id] != nullptr) 
    {
        return RecluseResult_Failed;
    }

    k_gpuBufferMap[id] = pBuffer;

    return RecluseResult_Ok;
}


ErrType cacheTexture2D(RenderID id, Texture2D* pTexture)
{
    if (isCachedTexture2D(id) && k_textureMap[id] != nullptr)
        return RecluseResult_Failed;

    k_textureMap[id] = pTexture;

    return RecluseResult_Ok;
}


Bool removeTexture2D(RenderID id)
{
    if (k_textureMap.find(id) == k_textureMap.end()) 
    {
        return false;
    }
    
    k_textureMap.erase(id);

    return true;
}


Bool removeGPUBuffer(RenderID id)
{
    if (k_gpuBufferMap.find(id) == k_gpuBufferMap.end()) 
    {
        return false;
    }
    
    k_gpuBufferMap.erase(id);

    return true;
}


Bool registerTexture2D(RenderID id)
{
    if (isCachedTexture2D(id)) 
    {
        return false;
    }

    k_textureMap[id] = nullptr;

    return true;
}


Bool registerGPUBuffer(RenderID id)
{
    if (isCachedGPUBuffer(id))
        return false;

    k_gpuBufferMap[id] = nullptr;
    
    return true;
}


Bool isCachedGPUBuffer(RenderID id)
{
    return k_gpuBufferMap.find(id) != k_gpuBufferMap.end();
}


Bool isCachedTexture2D(RenderID id)
{
    return k_textureMap.find(id) != k_textureMap.end();
}
} // RenderDB
} // Engine
} // Recluse