// 
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Types.hpp"

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
    ErrType initialize(Allocator* pAllocator, VulkanMemoryPool* poolRef) {
        m_allocator = pAllocator; 
        m_pool = poolRef;
        if (!m_allocator) return REC_RESULT_NULL_PTR_EXCEPTION;

        // Since we only need to work with offsets relative to the address,
        // We don't need the real address.
        m_allocator->initialize(0ull, poolRef->sizeBytes);

        return REC_RESULT_OK; 
    }

    ErrType allocate(VulkanMemory* pOut, VkMemoryRequirements& requirements);

    ErrType free(VulkanMemory* pOut);

    void destroy();

    void emptyGarbage() { }
    
private:

    U32 m_garbageIndex;
    std::vector<VulkanMemory> m_frameGarbage;
    VulkanMemoryPool*       m_pool;
    Allocator* m_allocator;
    // Get memory index from findMemoryType() function.
    U32         m_memoryIndex;
};
} // Recluse