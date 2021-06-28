// 
#pragma once

#include "VulkanCommons.hpp"
#include "Core/Types.hpp"

#include <vector>

namespace Recluse {

class VulkanDevice;

struct VulkanMemory {
    VkDeviceMemory  deviceMemory;
    VkDeviceSize    offsetBytes;
    VkDeviceSize    sizeBytes;
    void*           baseAddr;
};


class VulkanAllocator {
public:

    ErrType allocate(VulkanMemory* pOut, VkMemoryRequirements& requirements) { return REC_RESULT_NOT_IMPLEMENTED; }

    ErrType free(VulkanMemory* pOut) { return REC_RESULT_NOT_IMPLEMENTED; }

    void emptyGarbage() { }
    
private:

    U32 m_garbageIndex;
    std::vector<VulkanMemory> m_frameGarbage;
};
} // Recluse