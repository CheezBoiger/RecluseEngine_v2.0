//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"

#include "RecluseVulkan_exports.hpp"

#if defined (RECLUSE_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <vulkan/vulkan.h>

#define R_CHANNEL_VULKAN "Vulkan"
#ifndef VK_MAKE_API_VERSION
#define R_VULKAN_MAKE_API_VERSION(major, minor, patch) VK_MAKE_VERSION(major, minor, patch)
#else
#define R_VULKAN_MAKE_API_VERSION(major, minor, patch) VK_MAKE_API_VERSION(0, major, minor, patch)
#endif

#define R_RECLUSE_TARGET_VULKAN_API_VERSION() R_VULKAN_MAKE_API_VERSION(1, 1, 0)

// VK Version header must be equal, or above, this value, as an interface for ray tracing wasn't officially
// supported for some time. We might want to use the NVidia provided interface, but that might require some work.
#if (VK_HEADER_VERSION >= 236)
#define RECLUSE_RAYTRACING_HEADER
#endif


typedef void* VulkanMemoryPtr;

// Memory pool specified for vulkan, whenever we allocate, or suballocate, a memory chunk for a resource.
struct VulkanMemoryPool 
{
    VkDeviceMemory  memory;
    VkDeviceSize    sizeBytes;
    VulkanMemoryPtr basePtr;           
};

// Useful information about differences between D3D12 and Vulkan:
// https://asawicki.info/articles/memory_management_vulkan_direct3d_12.php5
// Proc access to these functions when we query for instance.
extern PFN_vkSetDebugUtilsObjectNameEXT    pfn_vkSetDebugUtilsObjectNameEXT;
extern PFN_vkSetDebugUtilsObjectTagEXT     pfn_vkSetDebugUtilsObjectTagEXT;
extern PFN_vkCmdBeginDebugUtilsLabelEXT    pfn_vkCmdBeginDebugUtilsLabelEXT;
extern PFN_vkCmdEndDebugUtilsLabelEXT      pfn_vkCmdEndDebugUtilsLabelEXT;

#if defined(VK_NV_mesh_shader)
extern PFN_vkCmdDrawMeshTasksNV            pfn_vkCmdDrawMeshTasksNV;
extern PFN_vkCmdDrawMeshTasksIndirectNV    pfn_vkCmdDrawMeshTasksIndirectNV;
#endif

#if defined(VK_EXT_mesh_shader)
extern PFN_vkCmdDrawMeshTasksEXT           pfn_vkCmdDrawMeshTasksEXT;
extern PFN_vkCmdDrawMeshTasksIndirectEXT   pfn_vkCmdDrawMeshTasksIndirectEXT;
#endif

#if defined(RECLUSE_RAYTRACING_HEADER)
extern PFN_vkCreateRayTracingPipelinesKHR  pfn_vkCreateRayTracingPipelinesKHR;
#endif

namespace Recluse {

class VulkanGraphicsObject : public virtual IGraphicsObject
{
public:
    virtual ~VulkanGraphicsObject() { }

    GraphicsAPI getApi() const override { return GraphicsApi_Vulkan; }
};

namespace Vulkan {

// The target api version for vulkan.
extern uint32_t                 targetApiVersion;
extern VkFormat                 getVulkanFormat(Recluse::ResourceFormat format);
extern Recluse::ResourceFormat  getResourceFormat(VkFormat format);
extern VkStencilOp              getNativeStencilOp(Recluse::StencilOp op);
extern uint32_t                 getFormatSizeBytes(VkFormat format);
extern VkImageAspectFlags       getAspectMask(VkFormat format);

static VkSampleCountFlagBits getSamples(Recluse::U32 count)
{
    switch (count) 
    {
        case 1: return VK_SAMPLE_COUNT_1_BIT;
        case 2: return VK_SAMPLE_COUNT_2_BIT;
        case 4: return VK_SAMPLE_COUNT_4_BIT;
        case 8: return VK_SAMPLE_COUNT_8_BIT;
        case 16: return VK_SAMPLE_COUNT_16_BIT;
        case 32: return VK_SAMPLE_COUNT_32_BIT;
        case 64: return VK_SAMPLE_COUNT_64_BIT;
        default: 
        {
            R_WARN(R_CHANNEL_VULKAN, "Sampling count is not supported (=%d), resorting to 1 sample count.", count);
            return VK_SAMPLE_COUNT_1_BIT;
        }
    }
}


extern VkShaderStageFlags getShaderStages(Recluse::ShaderStageFlags flags);


static VkCompareOp getNativeCompareOp(Recluse::CompareOp op) 
{
    switch (op) 
    {
        case Recluse::CompareOp_Always:
            return VK_COMPARE_OP_ALWAYS;
        case Recluse::CompareOp_Equal:
            return VK_COMPARE_OP_EQUAL;
        case Recluse::CompareOp_Greater:
            return VK_COMPARE_OP_GREATER;
        case Recluse::CompareOp_GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Recluse::CompareOp_Less:
            return VK_COMPARE_OP_LESS;
        case Recluse::CompareOp_LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Recluse::CompareOp_NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case Recluse::CompareOp_Never:
        default:
            return VK_COMPARE_OP_NEVER;
    }
}


static VkImageLayout getVulkanImageLayout(Recluse::ResourceState state)
{
    switch (state) 
    {
        case Recluse::ResourceState_Common:                 return VK_IMAGE_LAYOUT_GENERAL;
        case Recluse::ResourceState_UnorderedAccess:        return VK_IMAGE_LAYOUT_GENERAL;
        case Recluse::ResourceState_ShaderResource:         return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case Recluse::ResourceState_CopyDestination:        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case Recluse::ResourceState_CopySource:             return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case Recluse::ResourceState_RenderTarget:           return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case Recluse::ResourceState_DepthStencilReadOnly:   return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case Recluse::ResourceState_DepthStencilWrite:      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case Recluse::ResourceState_Present:                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default: return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}


static VkAccessFlags getDesiredResourceStateAccessMask(Recluse::ResourceState state)
{
    switch (state)
    {
        case Recluse::ResourceState_CopyDestination:         return VK_ACCESS_TRANSFER_WRITE_BIT;
        case Recluse::ResourceState_CopySource:              return VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;
        case Recluse::ResourceState_ConstantBuffer:          return VK_ACCESS_UNIFORM_READ_BIT;
        case Recluse::ResourceState_IndexBuffer:             return VK_ACCESS_INDEX_READ_BIT;
        case Recluse::ResourceState_DepthStencilWrite:       return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case Recluse::ResourceState_IndirectArgs:            return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        case Recluse::ResourceState_RenderTarget:            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        case Recluse::ResourceState_UnorderedAccess:         return VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        case Recluse::ResourceState_VertexBuffer:            return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        case Recluse::ResourceState_Common:                  return VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
        case Recluse::ResourceState_ShaderResource:          return VK_ACCESS_SHADER_READ_BIT;
        case Recluse::ResourceState_DepthStencilReadOnly:    return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        case Recluse::ResourceState_Present:                 return VK_ACCESS_MEMORY_READ_BIT;
    }

    return VK_ACCESS_MEMORY_READ_BIT;
}


static VkAccessFlags getDesiredHostMemoryUsageAccess(Recluse::ResourceMemoryUsage usage)
{
    switch (usage)
    {
        case Recluse::ResourceMemoryUsage_CpuVisible:      return VK_ACCESS_HOST_WRITE_BIT|VK_ACCESS_HOST_READ_BIT;
        case Recluse::ResourceMemoryUsage_CpuToGpu:     return VK_ACCESS_HOST_WRITE_BIT;
        case Recluse::ResourceMemoryUsage_GpuOnly:      return 0;
        case Recluse::ResourceMemoryUsage_GpuToCpu:     return VK_ACCESS_HOST_READ_BIT;
    }

    return 0;
}


static U32 unpackVulkanBinding(ShaderBind setBinding)
{
    return (setBinding & 0x0000ffff);
}


static U32 unpackVulkanSet(ShaderBind setBinding)
{
    return (setBinding & 0xffff0000);
}


static void unpackVulkanShaderSetBinding(ShaderBind setBinding, U32& set, U32& binding)
{
    set     = unpackVulkanSet(setBinding);
    binding = unpackVulkanBinding(binding);
}


struct BufferView 
{
    VkBuffer buffer;
    U32 offsetBytes;
    U16 sizeBytes;
    U16 pad0;
};


struct PhysicalDeviceFeaturesInfo
{
    VkPhysicalDeviceHostQueryResetFeatures  hostQueryResetFeatures;
#ifdef VK_EXT_mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesEXT   meshShaderFeaturesEXT;
#endif
#ifdef VK_NV_mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV    meshShaderFeaturesNV;
#endif
    VkPhysicalDeviceFeatures2               features2;

    PhysicalDeviceFeaturesInfo()
        : features2({VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2})
    {

        hostQueryResetFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES};

        push(hostQueryResetFeatures);

#ifdef VK_EXT_mesh_shader
        meshShaderFeaturesEXT = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
        push(meshShaderFeaturesEXT); 
#endif
#ifdef VK_NV_mesh_shader
        meshShaderFeaturesNV = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV};
        push(meshShaderFeaturesNV);
#endif
        //features2.pNext = &hostQueryResetFeatures;
    }

    PhysicalDeviceFeaturesInfo(const PhysicalDeviceFeaturesInfo& info)
    {
        features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        hostQueryResetFeatures = info.hostQueryResetFeatures;

#ifdef VK_EXT_mesh_shader
        meshShaderFeaturesEXT = info.meshShaderFeaturesEXT;
#endif

#ifdef VK_NV_mesh_shader
        meshShaderFeaturesNV = info.meshShaderFeaturesNV;
#endif

        features2.features = info.features2.features;

        push(hostQueryResetFeatures);

#ifdef VK_EXT_mesh_shader
        push(meshShaderFeaturesEXT);
#endif

#ifdef VK_NV_mesh_shader
        push(meshShaderFeaturesNV);
#endif
        //features2.pNext = &hostQueryResetFeatures;
    }

    template<typename Feature>
    void push(Feature& f)
    {
        f.pNext = features2.pNext;
        features2.pNext = &f;
    }
};
} // Vulkan
} // Recluse