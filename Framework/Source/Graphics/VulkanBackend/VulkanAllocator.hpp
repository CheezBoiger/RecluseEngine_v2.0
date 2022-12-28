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
        VulkanAllocUpdateFlag_None              = 0,
        VulkanAllocUpdateFlag_Update            = ( 1 << 0 ),
        VulkanAllocUpdateFlag_SetFrameIndex        = ( 1 << 1 ),
        VulkanAllocUpdateFlag_IncrementFrameIndex  = ( 1 << 2 ),
        VulkanAllocUpdateFlag_GarbageResize         = ( 1 << 3 ),
        VulkanAllocUpdateFlag_Clear                  = ( 1 << 4 )
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
        if (!m_allocator) return RecluseResult_NullPtrExcept;

        // Since we only need to work with offsets relative to the address,
        // We don't need the real address.
        m_allocator->initialize(0ull, poolRef->sizeBytes);

        m_frameGarbage.resize(garbageBufferCount);

        return RecluseResult_Ok; 
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
    void update(const UpdateConfig& config = { VulkanAllocUpdateFlag_None });

    // clear out the whole memory heap.
    void clear();
    
private:

    // Empty garbage from last frame.
    void emptyGarbage(U32 index);

    // Frame garbage holds copies of vulkan memory that needs to be cleaned up when
    // ready.
    ::std::vector<::std::vector<VulkanMemory>>  m_frameGarbage;

    VulkanMemoryPool*                       m_pool;
    Allocator*                              m_allocator;
    // Get memory index from findMemoryType() function.
    U32                                     m_memoryIndex;
    U32                                     m_garbageIndex;
};
} // Recluse