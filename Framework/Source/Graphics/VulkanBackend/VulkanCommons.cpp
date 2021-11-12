//
#include "VulkanCommons.hpp"

namespace Vulkan {


VkShaderStageFlags getShaderStages(Recluse::ShaderTypeFlags flags)
{
    VkShaderStageFlags vkFlags = 0;

    if (flags == Recluse::SHADER_TYPE_ALL) {
        vkFlags = VK_SHADER_STAGE_ALL;
    } else {
        if (flags & Recluse::SHADER_TYPE_PIXEL) {
            vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_VERTEX) {
            vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_HULL) {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_DOMAIN) {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_COMPUTE) {
            vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }

    return vkFlags;
}


VkFormat getVulkanFormat(Recluse::ResourceFormat format) {
  switch (format) {
    case Recluse::RESOURCE_FORMAT_B8G8R8A8_SRGB:
      return VK_FORMAT_B8G8R8A8_SRGB;
    case Recluse::RESOURCE_FORMAT_R8G8B8A8_UNORM:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case Recluse::RESOURCE_FORMAT_R16G16B16A16_FLOAT:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Recluse::RESOURCE_FORMAT_R11G11B10_FLOAT:
      return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case Recluse::RESOURCE_FORMAT_D32_FLOAT:
      return VK_FORMAT_D32_SFLOAT;
    case Recluse::RESOURCE_FORMAT_R32_FLOAT:
      return VK_FORMAT_R32_SFLOAT;
    case Recluse::RESOURCE_FORMAT_D24_UNORM_S8_UINT:
      return VK_FORMAT_D24_UNORM_S8_UINT;
    case Recluse::RESOURCE_FORMAT_R16G16_FLOAT:
      return VK_FORMAT_R16G16_SFLOAT;
    case Recluse::RESOURCE_FORMAT_R32G32B32A32_FLOAT:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Recluse::RESOURCE_FORMAT_R32G32B32A32_UINT:
      return VK_FORMAT_R32G32B32A32_UINT;
    case Recluse::RESOURCE_FORMAT_R32G32_FLOAT:
      return VK_FORMAT_R32G32_SFLOAT;
    case Recluse::RESOURCE_FORMAT_R32G32_UINT:
      return VK_FORMAT_R32G32_UINT;
    case Recluse::RESOURCE_FORMAT_D32_FLOAT_S8_UINT:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case Recluse::RESOURCE_FORMAT_R8_UINT:
      return VK_FORMAT_R8_UINT;
    case Recluse::RESOURCE_FORMAT_B8G8R8A8_UNORM:
      return VK_FORMAT_B8G8R8A8_UNORM;
    default:
      return VK_FORMAT_UNDEFINED;
  }
}
} // Vulkan