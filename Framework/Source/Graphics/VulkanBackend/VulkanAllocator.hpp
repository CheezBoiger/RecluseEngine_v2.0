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

    enum VulkanAllocUpdateFlag {
        VULKAN_ALLOC_NONE_FLAG          = 0,
        VULKAN_ALLOC_UPDATE_FLAG        = ( 1 << 0 ),
        VULKAN_ALLOC_SHIFT_FRAME_INDEX  = ( 1 << 1 )
    };

    typedef U32 VulkanAllocUpdateFlags;

    VulkanAllocator()
        : m_garbageIndex(0)
        , m_allocator(nullptr)
        , m_pool(nullptr) 
    { }

    ~VulkanAllocator() { }

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

    // Update allocator every new frame.
    void update(VulkanAllocUpdateFlags flags = VULKAN_ALLOC_NONE_FLAG);
    
private:

    // Empty garbage from last frame.
    void emptyGarbage();

    std::vector<std::vector<VulkanMemory>>  m_frameGarbage;
    VulkanMemoryPool*                       m_pool;
    Allocator*                              m_allocator;
    // Get memory index from findMemoryType() function.
    U32                                     m_memoryIndex;
    U32                                     m_garbageIndex;
};
} // Recluse