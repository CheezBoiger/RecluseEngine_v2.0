//
#include "VulkanShaderCache.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Types.hpp"

#include <unordered_map>

namespace Recluse {


struct ShaderHash {
    
};


VkResult createShaderModule(VulkanDevice* pDevice, Shader* pShader, VkShaderModule* pModule)
{
    if (!pShader || !pModule || !pDevice)
    {
        R_ERR(__FUNCTION__, "Either pShader, pModule, or pDevice were passed as NULL!! Can not create a VkShaderModule!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkShaderModule shaderModule     = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo info   = { };

    info.sType                      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pCode                      = (U32*)pShader->getByteCode();
    info.codeSize                   = (SizeT)pShader->getSzBytes();
    
    VkResult result                 = vkCreateShaderModule(pDevice->get(), &info, nullptr, &shaderModule);

    if (result != VK_SUCCESS)
    {
        R_ERR("VulkanShaderCache", "Failed (result = %d)", result);
    }
    else
    {
        *pModule = shaderModule;
    }

    return result;
}

VkShaderModule ShaderCache::getCachedShaderModule(VulkanDevice* pDevice, Shader* pShader)
{
    Hash64 hash = pShader->getHashId();

    if (isShaderCached(pShader)) 
    {
        return cache[hash];
    }

    cacheShader(pDevice, pShader);
    return cache[hash];
}


ErrType ShaderCache::cacheShader(VulkanDevice* pDevice, Shader* pShader)
{
    VkResult result             = VK_SUCCESS;
    VkShaderModule shaderModule = VK_NULL_HANDLE;

    if (!isShaderCached(pShader)) 
    {
        result = createShaderModule(pDevice, pShader, &shaderModule);
        
        if (result != VK_SUCCESS) 
        {
            return RecluseResult_Failed;
        }
        
        cache[pShader->getHashId()] = shaderModule;
    }

    return RecluseResult_Ok;
}


B32 ShaderCache::isShaderCached(Shader* pShader)
{
    return (cache.find(pShader->getHashId()) != cache.end());
}


void ShaderCache::clearCache(VulkanDevice* pDevice)
{
    for (auto keyval : cache) 
    {
        if (keyval.second) 
        {
            vkDestroyShaderModule(pDevice->get(), keyval.second, nullptr);
        }
    }

    cache.clear();
}
} // Recluse