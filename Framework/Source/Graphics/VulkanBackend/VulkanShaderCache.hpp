//
#pragma once

#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"

#include <unordered_map>

namespace Recluse {

class Shader;
class VulkanDevice;


class ShaderCache 
{
public:
    // Get the shader cache module. This is the native vulkan module.
    VkShaderModule  getCachedShaderModule(VulkanDevice* pDevice, Shader* pShader);

    // Cache the shader, this also checks if a VkShaderModule is also present. If not,
    // Will create one.
    ErrType         cacheShader(VulkanDevice* pDevice, Shader* pShader);

    // Check if the given shader has already been cached.
    B32             isShaderCached(Shader* pShader);

    // Clear the whole cache from this shader. Should be called when app closes!
    void            clearCache(VulkanDevice* pDevice);

    // Uncache an individual shader. This will destroy the VkShaderModule!
    ErrType         unCacheShader(VulkanDevice* pDevice, Shader* pShader);

private:
    // Cache holds all system cache info.
    std::unordered_map<Hash64, VkShaderModule> cache;
};
} // Recluse