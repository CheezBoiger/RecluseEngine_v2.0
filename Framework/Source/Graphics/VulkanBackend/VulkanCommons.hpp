//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

#if defined (RECLUSE_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <vulkan/vulkan.h>

#define R_CHANNEL_VULKAN "Vulkan"

#define SETBIND(bindType, var) switch (bindType) { \
    case BIND_TYPE_COMPUTE: var = VK_PIPELINE_BIND_POINT_COMPUTE; break; \
    case BIND_TYPE_RAY_TRACE: var = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR; break; \
    case BIND_TYPE_GRAPHICS: \
    default: var = VK_PIPELINE_BIND_POINT_GRAPHICS; break; \
}

struct VulkanMemoryPool {
    VkDeviceMemory  memory;
    VkDeviceSize    sizeBytes;
    void*           basePtr;           
};

namespace Vulkan {

extern VkFormat getVulkanFormat(Recluse::ResourceFormat format);


static VkSampleCountFlagBits getSamples(Recluse::U32 count)
{
    switch (count) {
        case 1: return VK_SAMPLE_COUNT_1_BIT;
        case 2: return VK_SAMPLE_COUNT_2_BIT;
        case 4: return VK_SAMPLE_COUNT_4_BIT;
        case 8: return VK_SAMPLE_COUNT_8_BIT;
        case 16: return VK_SAMPLE_COUNT_16_BIT;
        case 32: return VK_SAMPLE_COUNT_32_BIT;
        case 64: return VK_SAMPLE_COUNT_64_BIT;
        default: return VK_SAMPLE_COUNT_1_BIT;
    }
}


extern VkShaderStageFlags getShaderStages(Recluse::ShaderTypeFlags flags);
} // Vulkan