//
#pragma once

#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"


namespace Recluse {

class Shader;
class VulkanDevice;


namespace ShaderCache {


VkShaderModule getCachedShaderModule(VulkanDevice* pDevice, Shader* pShader);
ErrType cacheShader(VulkanDevice* pDevice, Shader* pShader);
B32 isShaderCached(Shader* pShader);
void clearCache(VulkanDevice* pDevice);
} // ShaderCache
} // Recluse