// 
#pragma once

#include "VulkanCommons.hpp"
#include "Core/Types.hpp"

namespace Recluse {

class VulkanDevice;

struct VulkanMemory {
    VkDeviceMemory  deviceMemory;
    VkDeviceSize    offsetBytes;
    VkDeviceSize    sizeBytes;
};


class VulkanAllocator {
public:

    ErrType allocate(VulkanMemory* pOut, VkMemoryRequirements& requirements) { return REC_RESULT_NOT_IMPLEMENTED; }

    ErrType free(VulkanMemory* pOut) { return REC_RESULT_NOT_IMPLEMENTED; }
};
} // Recluse