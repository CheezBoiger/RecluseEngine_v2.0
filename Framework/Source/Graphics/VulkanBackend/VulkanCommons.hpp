//
#pragma once

#include "Core/Arch.hpp"
#include "Core/Types.hpp"
#include "Graphics/Format.hpp"

#if defined (RECLUSE_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <vulkan/vulkan.h>

#define R_CHANNEL_VULKAN "Vulkan"

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
} // Vulkan