//
#include "VulkanCommons.hpp"

// Set to null, but will be queried as soon as we get an instance, and we are supporting debug utils.
PFN_vkSetDebugUtilsObjectNameEXT    pfn_vkSetDebugUtilsObjectNameEXT    = nullptr;
PFN_vkSetDebugUtilsObjectTagEXT     pfn_vkSetDebugUtilsObjectTagEXT     = nullptr;

namespace Vulkan {


VkShaderStageFlags getShaderStages(Recluse::ShaderStageFlags flags)
{
    VkShaderStageFlags vkFlags = 0;

    if (flags == Recluse::ShaderStage_All) 
    {
        vkFlags = VK_SHADER_STAGE_ALL;
    } 
    else 
    {
        if (flags & Recluse::ShaderStage_Pixel) 
        {
            vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (flags & Recluse::ShaderStage_Vertex) 
        {
            vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (flags & Recluse::ShaderStage_Hull) 
        {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (flags & Recluse::ShaderStage_Domain) 
        {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (flags & Recluse::ShaderStage_Compute) 
        {
            vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }

    return vkFlags;
}


// TODO: This will get tedious overtime, so we might want something that will gracefully 
//       map this. (We could use two maps for this.)

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


Recluse::ResourceFormat getResourceFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
        return Recluse::ResourceFormat_R8G8B8A8_Unorm;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return Recluse::ResourceFormat_R16G16B16A16_Float;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return Recluse::ResourceFormat_R11G11B10_Float;
    case VK_FORMAT_D32_SFLOAT:
        return Recluse::ResourceFormat_D32_Float;
    case VK_FORMAT_R32_SFLOAT:
        return Recluse::ResourceFormat_R32_Float;
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return Recluse::ResourceFormat_D24_Unorm_S8_Uint;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return Recluse::ResourceFormat_D32_Float_S8_Uint;
    case VK_FORMAT_R16G16_SFLOAT:
        return Recluse::ResourceFormat_R16G16_Float;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return Recluse::ResourceFormat_B8G8R8A8_Srgb;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return Recluse::ResourceFormat_R32G32B32A32_Float;
    case VK_FORMAT_R32G32B32A32_UINT:
        return Recluse::ResourceFormat_R32G32B32A32_Uint;
    case VK_FORMAT_R8_UINT:
        return Recluse::ResourceFormat_R8_Uint;
    case VK_FORMAT_R32G32_SFLOAT:
        return Recluse::ResourceFormat_R32G32_Float;
    case VK_FORMAT_R32G32_UINT:
        return Recluse::ResourceFormat_R32G32_Uint;
    case VK_FORMAT_R16_UINT:
        return Recluse::ResourceFormat_R16_Uint;
    case VK_FORMAT_R16_SFLOAT:
        return Recluse::ResourceFormat_R16_Uint;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return Recluse::ResourceFormat_B8G8R8A8_Unorm;
    default:
        return Recluse::ResourceFormat_Unknown;
    }
}
} // Vulkan