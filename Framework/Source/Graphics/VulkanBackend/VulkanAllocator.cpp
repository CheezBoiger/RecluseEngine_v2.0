//
#include "VulkanAllocator.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


B32 isMemoryResourcesOnSeparatePages(VkDeviceSize offsetA, 
    VkDeviceSize sizeA,     
    VkDeviceSize offsetB, 
    VkDeviceSize sizeB, 
    VkDeviceSize bufferImageGranularity)
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
    ErrType result          = REC_RESULT_OK;
    PtrType baseAddr        = m_allocator->getBaseAddr();

    result = m_allocator->allocate(&allocation, requirements.size, (U16)requirements.alignment);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate memory!");

        return result;
    }

    pOut->sizeBytes     = allocation.sizeBytes;
    pOut->offsetBytes   = allocation.ptr;
    pOut->baseAddr      = m_pool->basePtr;
    pOut->deviceMemory  = m_pool->memory;
    
    return result;
}


ErrType VulkanAllocator::free(VulkanMemory* pOut)
{
    R_ASSERT(m_allocator != NULL);
    
    ErrType result = REC_RESULT_OK;    

    if (!pOut) {
    
        return REC_RESULT_NULL_PTR_EXCEPTION;
    
    }

    Allocation allocation   = { };
    allocation.ptr          = pOut->offsetBytes;
    allocation.sizeBytes    = pOut->sizeBytes;

    result = m_allocator->free(&allocation);
    
    return result;
}


void VulkanAllocator::destroy()
{
    if (m_allocator) {

        m_allocator->cleanUp();

        delete m_allocator;
        m_allocator = nullptr;

    }
}


void VulkanAllocator::emptyGarbage()
{
    std::vector<VulkanMemory>& garbage = m_frameGarbage[m_garbageIndex];
    ErrType result;

    for (U32 i = 0; i < m_frameGarbage.size(); ++i) {
        result = REC_RESULT_OK;
        
        VulkanMemory& mrange = garbage[i];
        Allocation alloc = { };
        alloc.ptr = mrange.offsetBytes;
        alloc.sizeBytes = mrange.sizeBytes;

        result = m_allocator->free(&alloc);

        if (result != REC_RESULT_FAILED) {
            R_ERR("Allocator", "Failed to free garbage addr=%llu, at base addr=%llu", 
                (U64)mrange.baseAddr + mrange.offsetBytes, (U64)mrange.baseAddr);
        }
    }

    garbage.clear();
}


void VulkanAllocator::update(VulkanAllocUpdateFlags flags)
{
    if (flags & VULKAN_ALLOC_UPDATE_FLAG) {
        emptyGarbage();
    }

    if (flags & VULKAN_ALLOC_SHIFT_FRAME_INDEX) {
        U32 garbageSize = (U32)m_frameGarbage.size();
        m_garbageIndex = (m_garbageIndex + 1) % garbageSize;
    }
}
} // Recluse