//
#include "VulkanAllocator.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


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
} // Recluse