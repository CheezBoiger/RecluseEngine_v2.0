//
#include "VulkanShaderCache.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Types.hpp"

#include <unordered_map>

namespace Recluse {
namespace ShaderCache {


std::unordered_map<Hash64, VkShaderModule> cache;


struct ShaderHash {
    
};

VkShaderModule getCachedShaderModule(VulkanDevice* pDevice, Shader* pShader)
{
    Hash64 hash = pShader->getCrc();

    if (isShaderCached(pShader)) {
        return cache[hash];
    }

    cacheShader(pDevice, pShader);
    return cache[hash];
}


ErrType cacheShader(VulkanDevice* pDevice, Shader* pShader)
{
    VkResult result             = VK_SUCCESS;
    VkShaderModule shaderModule = VK_NULL_HANDLE;

    if (!isShaderCached(pShader)) {
        VkShaderModuleCreateInfo info = { };
        info.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pCode      = (uint32_t*)pShader->getByteCode();
        info.codeSize   = (size_t)pShader->getSzBytes();
        result = vkCreateShaderModule(pDevice->get(), &info, nullptr, &shaderModule);
        if (result != VK_SUCCESS) {
            return REC_RESULT_FAILED;
        }
        
        cache[pShader->getCrc()] = shaderModule;
    }

    return REC_RESULT_OK;
}


B32 isShaderCached(Shader* pShader)
{
    return (cache.find(pShader->getCrc()) != cache.end());
}


void clearCache(VulkanDevice* pDevice)
{
    for (auto keyval : cache) {
        if (keyval.second) {
            vkDestroyShaderModule(pDevice->get(), keyval.second, nullptr);
        }
    }

    cache.clear();
}
} // ShaderCache
} // Recluse