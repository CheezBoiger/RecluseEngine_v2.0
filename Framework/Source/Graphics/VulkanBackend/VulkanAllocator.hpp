// 
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Types.hpp"

#include <vector>

namespace Recluse {

class VulkanDevice;

struct VulkanMemory 
{
    VkDeviceMemory  deviceMemory;
    VkDeviceSize    offsetBytes;
    VkDeviceSize    sizeBytes;
    void*           baseAddr;
};


// Vulkan allocator manager, handles memory heaps provided by Vulkan API.
//
class VulkanAllocator 
{
public:

    enum VulkanAllocUpdateFlag 
    {
        VULKAN_ALLOC_NONE_FLAG              = 0,
        VULKAN_ALLOC_UPDATE_FLAG            = ( 1 << 0 ),
        VULKAN_ALLOC_SET_FRAME_INDEX        = ( 1 << 1 ),
        VULKAN_ALLOC_INCREMENT_FRAME_INDEX  = ( 1 << 2 ),
        VULKAN_ALLOC_GARBAGE_RESIZE         = ( 1 << 3 ),
        VULKAN_ALLOC_CLEAR                  = ( 1 << 4 )
    };

    typedef U32 VulkanAllocUpdateFlags;

    struct UpdateConfig 
    {
        VulkanAllocUpdateFlags flags;
        U32 frameIndex          : 16;
        U32 garbageBufferCount  : 16;
    };

    VulkanAllocator()
        : m_garbageIndex(0)
        , m_allocator(nullptr)
        , m_pool(nullptr) 
    { }

    ~VulkanAllocator() { }

    ErrType initialize
                (
                    Allocator* pAllocator, 
                    VulkanMemoryPool* poolRef,
                    U32 garbageBufferCount
                ) 
    {
        m_allocator = pAllocator; 
        m_pool = poolRef;
        if (!m_allocator) return R_RESULT_NULL_PTR_EXCEPT;

        // Since we only need to work with offsets relative to the address,
        // We don't need the real address.
        m_allocator->initialize(0ull, poolRef->sizeBytes);

        m_frameGarbage.resize(garbageBufferCount);

        return R_RESULT_OK; 
    }

    // Allocate memory from program managed heap. This will require 
    // using automated allocation structures.
    // @param pOut 
    // @param requirements
    // 
    // @return ErrType result.
    ErrType allocate(VulkanMemory* pOut, VkMemoryRequirements& requirements);

    // Free up memory, handles throwing memory into the garbage.
    // Immediate free can be done if we know we are in the same frame index.
    ErrType free(VulkanMemory* pOut, Bool immediate = false);

    // Destroy this allocation manager from the inside!
    void destroy();

    // Update allocator every new frame.
    void update(const UpdateConfig& config = { VULKAN_ALLOC_NONE_FLAG });

    // clear out the whole memory heap.
    void clear();
    
private:

    // Empty garbage from last frame.
    void emptyGarbage(U32 index);

    // Frame garbage holds copies of vulkan memory that needs to be cleaned up when
    // ready.
    std::vector<std::vector<VulkanMemory>>  m_frameGarbage;

    VulkanMemoryPool*                       m_pool;
    Allocator*                              m_allocator;
    // Get memory index from findMemoryType() function.
    U32                                     m_memoryIndex;
    U32                                     m_garbageIndex;
};
} // Recluse