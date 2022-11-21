//
#include "VulkanAllocator.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


static B32 isMemoryResourcesOnSeparatePages
                (
                    VkDeviceSize offsetA, 
                    VkDeviceSize sizeA,     
                    VkDeviceSize offsetB, 
                    VkDeviceSize sizeB, 
                    VkDeviceSize bufferImageGranularity
                )
{
    VkDeviceSize endA       = offsetA + sizeA - 1;
    VkDeviceSize endPageA   = endA & ~(bufferImageGranularity - 1);
    VkDeviceSize startB     = offsetB;
    VkDeviceSize startPageB = startB & ~(bufferImageGranularity - 1);

    return endPageA < startPageB;
}


ErrType VulkanAllocator::allocate(VulkanMemory* pOut, VkMemoryRequirements& requirements)
{
    R_ASSERT(m_allocator != NULL);
    
    Allocation allocation   = { };
    ErrType result          = RecluseResult_Ok;
    PtrType baseAddr        = m_allocator->getBaseAddr();

    result = m_allocator->allocate(&allocation, requirements.size, (U16)requirements.alignment);
    
    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate memory!");

        return result;
    }

    pOut->sizeBytes     = allocation.sizeBytes;
    pOut->offsetBytes   = allocation.baseAddress;
    pOut->baseAddr      = m_pool->basePtr;
    pOut->deviceMemory  = m_pool->memory;
    
    return result;
}


ErrType VulkanAllocator::free(VulkanMemory* pOut, Bool immediate)
{
    R_ASSERT(m_allocator != NULL);
    
    ErrType result = RecluseResult_Ok;    

    if (!pOut) 
    {
        return RecluseResult_NullPtrExcept;
    }

    if (immediate) 
    {
        Allocation allocation   = { };
        allocation.baseAddress          = pOut->offsetBytes;
        allocation.sizeBytes    = pOut->sizeBytes;

        result = m_allocator->free(&allocation);
    } 
    else 
    {
        // Push back a copy.
        std::vector<VulkanMemory>& garbageChute = m_frameGarbage[m_garbageIndex];
        garbageChute.push_back(*pOut);
    }

    return result;
}


void VulkanAllocator::destroy()
{
    // Perform a full garbage cleanup.
    for (U32 i = 0; i < m_frameGarbage.size(); ++i) 
    {
        emptyGarbage(i);
    }

    m_frameGarbage.clear();

    if (m_allocator) 
    {

        m_allocator->cleanUp();

        delete m_allocator;
        m_allocator = nullptr;

    }
}


void VulkanAllocator::emptyGarbage(U32 index)
{
    std::vector<VulkanMemory>& garbage = m_frameGarbage[index];
    ErrType result;

    for (U32 i = 0; i < garbage.size(); ++i) 
    {
        result = RecluseResult_Ok;
        
        VulkanMemory& mrange    = garbage[i];
        Allocation alloc        = { };

        alloc.baseAddress               = mrange.offsetBytes;
        alloc.sizeBytes         = mrange.sizeBytes;

        result = m_allocator->free(&alloc);

        if (result != RecluseResult_Ok) 
        {
            R_ERR
                (
                    "Allocator", 
                    "Failed to free garbage addr=%llu, at base addr=%llu", 
                    (U64)mrange.baseAddr + mrange.offsetBytes, 
                    (U64)mrange.baseAddr
                );
        }
    }

    garbage.clear();
}


void VulkanAllocator::update(const UpdateConfig& config) 
{
    if 
        (
            (config.flags & VULKAN_ALLOC_GARBAGE_RESIZE) 
            && (config.garbageBufferCount != (U32)m_frameGarbage.size())
        ) 
    {
        const U32 newGarbageCount = config.garbageBufferCount;
        // Clean up all garbage before doing this...
        R_DEBUG("VulkanAllocator", "Updating garbage buffers...");
        for (U32 i = 0; i < (U32)m_frameGarbage.size(); ++i) 
        {
            emptyGarbage(i);
        }
        
        m_frameGarbage.resize(newGarbageCount);
        R_DEBUG("VulkanAllocator", "Finished");
    }

    U32 garbageSize = (U32)m_frameGarbage.size();
        
    if (config.flags & VULKAN_ALLOC_SET_FRAME_INDEX) 
    {
        if (config.frameIndex >= garbageSize) 
        {
            R_ERR
                (
                    "VulkanAllocator", 
                    "frameIndex to set (=%d), is larger than the number "
                    "of garbage frames (=%d)! Ignoring set...", 
                    config.frameIndex, 
                    garbageSize
                );
        } 
        else 
        {
            m_garbageIndex = config.frameIndex;
        }
    } 
    else if (config.flags & VULKAN_ALLOC_INCREMENT_FRAME_INDEX) 
    {
        m_garbageIndex = (m_garbageIndex + 1) % garbageSize;
    }

    if (config.flags & VULKAN_ALLOC_UPDATE_FLAG)
    {
        emptyGarbage(m_garbageIndex);
    }
}


void VulkanAllocator::clear()
{
    R_ASSERT(m_pool->memory != NULL);
    R_ASSERT(m_allocator    != NULL);

    for (U32 i = 0; i < (U32)m_frameGarbage.size(); ++i) 
    {
      m_frameGarbage[i].clear();
    }

    m_allocator->reset();
}
} // Recluse