//
#include "VulkanCommons.hpp"

// Set to null, but will be queried as soon as we get an instance, and we are supporting debug utils.
PFN_vkSetDebugUtilsObjectNameEXT    pfn_vkSetDebugUtilsObjectNameEXT    = nullptr;
PFN_vkSetDebugUtilsObjectTagEXT     pfn_vkSetDebugUtilsObjectTagEXT     = nullptr;

#if defined (RECLUSE_RAYTRACING_HEADER)
PFN_vkCreateRayTracingPipelinesKHR  pfn_vkCreateRayTracingPipelinesKHR  = nullptr;
#endif

namespace Recluse {
namespace Vulkan {


VkStencilOp getNativeStencilOp(Recluse::StencilOp op)
{
    switch (op)
    {
        case Recluse::StencilOp_DecrementAndClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case Recluse::StencilOp_DecrementAndWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case Recluse::StencilOp_IncrementAndClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case Recluse::StencilOp_IncrementAndWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case Recluse::StencilOp_Invert:
            return VK_STENCIL_OP_INVERT;
        case Recluse::StencilOp_Keep:
            return VK_STENCIL_OP_KEEP;
        case Recluse::StencilOp_Replace:
            return VK_STENCIL_OP_REPLACE;
        case Recluse::StencilOp_Zero:
        default: return VK_STENCIL_OP_ZERO;
    }
}

uint32_t getFormatSizeBytes(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_UINT:

            return 16u;

        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_UINT:

            return 12u;

        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_UINT:

            return 8u;

        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D24_UNORM_S8_UINT:

            return 4u;

        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        
            return 3u;

        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_USCALED:

            return 2u;

        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_S8_UINT:
            
            return 1u;
    }

    return 0u;
}


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
        if (flags & Recluse::ShaderStage_Mesh)
        {
            vkFlags |= VK_SHADER_STAGE_MESH_BIT_NV;
        }
        if (flags & Recluse::ShaderStage_Task)
        {
            vkFlags |= VK_SHADER_STAGE_TASK_BIT_NV;
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
        case Recluse::ResourceFormat_R32G32B32_Float:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case Recluse::ResourceFormat_R32_Uint:
            return VK_FORMAT_R32_UINT;
        case Recluse::ResourceFormat_R32_Int:
            return VK_FORMAT_R32_SINT;
        case Recluse::ResourceFormat_BC1_Unorm:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case Recluse::ResourceFormat_BC2_Unorm:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case Recluse::ResourceFormat_BC3_Unorm:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case Recluse::ResourceFormat_BC4_Unorm:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case Recluse::ResourceFormat_BC5_Unorm:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case Recluse::ResourceFormat_BC7_Unorm:
            return VK_FORMAT_BC7_UNORM_BLOCK;
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
    case VK_FORMAT_R32G32B32_SFLOAT:
        return Recluse::ResourceFormat_R32G32B32_Float;
    case VK_FORMAT_R32_UINT:
        return Recluse::ResourceFormat_R32_Uint;
    case VK_FORMAT_R32_SINT:
        return Recluse::ResourceFormat_R32_Int;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        return Recluse::ResourceFormat_BC1_Unorm;
    case VK_FORMAT_BC2_UNORM_BLOCK:
        return Recluse::ResourceFormat_BC2_Unorm;
    case VK_FORMAT_BC3_UNORM_BLOCK:
        return Recluse::ResourceFormat_BC3_Unorm;
    case VK_FORMAT_BC4_UNORM_BLOCK:
        return Recluse::ResourceFormat_BC4_Unorm;
    case VK_FORMAT_BC5_UNORM_BLOCK:
        return Recluse::ResourceFormat_BC5_Unorm;
    case VK_FORMAT_BC7_UNORM_BLOCK:
        return Recluse::ResourceFormat_BC7_Unorm;
    default:
        return Recluse::ResourceFormat_Unknown;
    }
}


VkImageAspectFlags getAspectMask(VkFormat format)
{
    VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT; 
    // If we have depth and stencil, check here.
    switch (format)
    {
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
            flags = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            flags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_S8_UINT:
            flags = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
    }

    return flags;
}
} // Vulkan
} // Recluse

#if defined(RECLUSE_RAYTRACING_HEADER)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines)
{
    return pfn_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
#endif