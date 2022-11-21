//
#include "VulkanCommons.hpp"

namespace Vulkan {


VkShaderStageFlags getShaderStages(Recluse::ShaderTypeFlags flags)
{
    VkShaderStageFlags vkFlags = 0;

    if (flags == Recluse::ShaderType_All) 
    {
        vkFlags = VK_SHADER_STAGE_ALL;
    } 
    else 
    {
        if (flags & Recluse::ShaderType_Pixel) 
        {
            vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (flags & Recluse::ShaderType_Vertex) 
        {
            vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (flags & Recluse::ShaderType_Hull) 
        {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (flags & Recluse::ShaderType_Domain) 
        {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (flags & Recluse::ShaderType_Compute) 
        {
            vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }

    return vkFlags;
}


VkFormat getVulkanFormat(Recluse::ResourceFormat format) 
{
    switch (format) 
    {
        case Recluse::ResourceFormat_B8G8R8A8_Srgb:
          return VK_FORMAT_B8G8R8A8_SRGB;
        case Recluse::ResourceFormat_R8G8B8A8_Unorm:
          return VK_FORMAT_R8G8B8A8_UNORM;
        case Recluse::ResourceFormat_R16G16B16A16_Float:
          return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Recluse::ResourceFormat_R11G11B10_Float:
          return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case Recluse::ResourceFormat_D32_Float:
          return VK_FORMAT_D32_SFLOAT;
        case Recluse::ResourceFormat_R32_Float:
          return VK_FORMAT_R32_SFLOAT;
        case Recluse::ResourceFormat_D24_Unorm_S8_Uint:
          return VK_FORMAT_D24_UNORM_S8_UINT;
        case Recluse::ResourceFormat_R16G16_Float:
          return VK_FORMAT_R16G16_SFLOAT;
        case Recluse::ResourceFormat_R32G32B32A32_Float:
          return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Recluse::ResourceFormat_R32G32B32A32_Uint:
          return VK_FORMAT_R32G32B32A32_UINT;
        case Recluse::ResourceFormat_R32G32_Float:
          return VK_FORMAT_R32G32_SFLOAT;
        case Recluse::ResourceFormat_R32G32_Uint:
          return VK_FORMAT_R32G32_UINT;
        case Recluse::ResourceFormat_D32_Float_S8_Uint:
          return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Recluse::ResourceFormat_R8_Uint:
          return VK_FORMAT_R8_UINT;
        case Recluse::ResourceFormat_B8G8R8A8_Unorm:
          return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            return VK_FORMAT_UNDEFINED;
    }
}
} // Vulkan