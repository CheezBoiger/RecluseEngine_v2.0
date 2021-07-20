//
#pragma once

#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"

#include <unordered_map>

namespace Recluse {

class Shader;
class VulkanDevice;


class ShaderCache {
public:

    VkShaderModule getCachedShaderModule(VulkanDevice* pDevice, Shader* pShader);
    ErrType cacheShader(VulkanDevice* pDevice, Shader* pShader);
    B32 isShaderCached(Shader* pShader);
    void clearCache(VulkanDevice* pDevice);

private:
    std::unordered_map<Hash64, VkShaderModule> cache;
};
} // Recluse