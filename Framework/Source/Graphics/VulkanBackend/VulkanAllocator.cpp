//
#include "VulkanAdapter.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {

VkDeviceSize VulkanAllocationManager::kPerMemoryPageSizeBytes = R_MB(8);

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


ErrType VulkanPagedAllocator::allocate(VulkanMemory* pOut, const VkMemoryRequirements& requirements)
{
    R_ASSERT(m_allocator != NULL);
    
    Allocation allocation   = { };
    ErrType result          = RecluseResult_Ok;
    PtrType baseAddr        = m_allocator->getBaseAddr();

    result = m_allocator->allocate(&allocation, requirements.size, (U16)requirements.alignment);
    
    if (result == RecluseResult_OutOfMemory)
    {
        return result;
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate memory!");

        return result;
    }

    pOut->sizeBytes         = allocation.sizeBytes;
    pOut->offsetBytes       = allocation.baseAddress;
    pOut->deviceMemory      = m_pool.memory;
    pOut->baseAddr          = m_pool.basePtr;
    return result;
}


ErrType VulkanPagedAllocator::free(VulkanMemory* pOut)
{
    R_ASSERT(m_allocator != NULL);
    
    ErrType result = RecluseResult_Ok;    

    if (!pOut) 
    {
        return RecluseResult_NullPtrExcept;
    }

    Allocation allocation   = { };
    allocation.baseAddress  = pOut->offsetBytes;
    allocation.sizeBytes    = pOut->sizeBytes;

    result = m_allocator->free(&allocation);
    //else 
    //{
    //    // Push back a copy.
    //    std::vector<VulkanMemory>& garbageChute = m_frameGarbage[m_garbageIndex];
    //    garbageChute.push_back(*pOut);
    //}

    return result;
}


Bool VulkanPagedAllocator::hasSpace(VkDeviceSize requestedSize) const
{
    const U64 totalSizeBytes = m_allocator->getTotalSizeBytes();
    const U64 usedSizeBytes = m_allocator->getUsedSizeBytes();
    return ((usedSizeBytes + requestedSize) < totalSizeBytes);
}


void VulkanPagedAllocator::release(VkDevice device)
{
    if (m_allocator) 
    {

        m_allocator->cleanUp();

        delete m_allocator;
        m_allocator = nullptr;
    }

    if (m_pool.memory)
    {
        vkFreeMemory(device, m_pool.memory, nullptr);
        m_pool.memory = VK_NULL_HANDLE;
    }
}


ErrType VulkanAllocationManager::release()
{
    VkDevice device = m_pDevice->get();
    // Perform a full garbage cleanup.
    for (U32 i = 0; i < m_frameGarbage.size(); ++i)
    {
        emptyGarbage(i);
    }

    m_frameGarbage.clear();

    for (auto it : m_resourceAllocators)
    {
        for (auto allocator : it.second)
            allocator->release(device);
    }

    return RecluseResult_Ok;
}


void VulkanAllocationManager::emptyGarbage(U32 index)
{
    std::vector<VulkanMemory>& garbage = m_frameGarbage[index];
    ErrType result;

    for (U32 i = 0; i < garbage.size(); ++i) 
    {
        result = RecluseResult_Ok;
        
        VulkanMemory& mrange        = garbage[i];
        Allocation alloc            = { };
        VulkanPagedAllocator* allocator  = m_resourceAllocators[mrange.memoryTypeIndex][mrange.allocatorIndex];

        alloc.baseAddress       = mrange.offsetBytes;
        alloc.sizeBytes         = mrange.sizeBytes;

        result = allocator->free(&mrange);

        if (result != RecluseResult_Ok) 
        {
            const U64 baseAddress = reinterpret_cast<U64>(mrange.baseAddr);
            R_ERR
                (
                    "Allocator", 
                    "Failed to free garbage addr=%llu, at base addr=%llu", 
                    (U64)baseAddress + mrange.offsetBytes,
                    (U64)baseAddress
                );
        }
    }

    garbage.clear();
}


void VulkanAllocationManager::update(const UpdateConfig& config) 
{
    if 
        (
            (config.flags & VulkanAllocUpdateFlag_GarbageResize) 
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
        
    if (config.flags & VulkanAllocUpdateFlag_SetFrameIndex) 
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
    else if (config.flags & VulkanAllocUpdateFlag_IncrementFrameIndex) 
    {
        m_garbageIndex = (m_garbageIndex + 1) % garbageSize;
    }

    if (config.flags & VulkanAllocUpdateFlag_Update)
    {
        emptyGarbage(m_garbageIndex);
    }
}

void VulkanPagedAllocator::clear()
{
    R_ASSERT(m_pool.memory != VK_NULL_HANDLE);
    R_ASSERT(m_allocator    != NULL);

    m_allocator->reset();
}


ErrType VulkanAllocationManager::initialize(VulkanDevice* device)
{
    m_pDevice = device;
    return RecluseResult_Ok;
}


ErrType VulkanAllocationManager::allocateBuffer(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements)
{
    VkDevice device                     = m_pDevice->get();
    VulkanAdapter* pAdapter             = m_pDevice->getAdapter();
    MemoryTypeIndex memoryTypeIndex     = pAdapter->findMemoryType(requirements.memoryTypeBits, usage);
    VulkanPagedAllocator* pAllocator    = nullptr;

    // TODO: We need to set the limit of page allocations are allowed per request.
    
    auto it = m_resourceAllocators.find(memoryTypeIndex);
    if (it == m_resourceAllocators.end())
    {
        pAllocator = allocateMemoryPage(memoryTypeIndex, usage);
    }
    else
    {
        for (U32 i = 0; i < m_resourceAllocators[memoryTypeIndex].size(); ++i)
        {
            VulkanPagedAllocator* potentialAllocator = m_resourceAllocators[memoryTypeIndex][i];
            if (potentialAllocator->hasSpace(align(requirements.size, requirements.alignment)))
            {
                pAllocator = potentialAllocator;
                break;
            }
        }
    }

    if (!pAllocator)
    {
        pAllocator = allocateMemoryPage(memoryTypeIndex, usage);
    }

    return pAllocator->allocate(pOut, requirements);
}


ErrType VulkanAllocationManager::allocateImage(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling)
{
    return RecluseResult_NoImpl;
}


ErrType VulkanAllocationManager::free(VulkanMemory* pOut, Bool immediate)
{
    std::vector<VulkanMemory>& garbageChute = m_frameGarbage[m_garbageIndex];
    garbageChute.push_back(*pOut);
    return RecluseResult_Ok;
}


VulkanPagedAllocator* VulkanAllocationManager::allocateMemoryPage(MemoryTypeIndex memoryTypeIndex, ResourceMemoryUsage usage)
{
    VkDevice device                     = m_pDevice->get();
    m_resourceAllocators[memoryTypeIndex].push_back(makeSmartPtr(new VulkanPagedAllocator()));
    VulkanPagedAllocator* pAllocator    = m_resourceAllocators[memoryTypeIndex].back();
    pAllocator->initialize(device, new LinearAllocator(), memoryTypeIndex, kPerMemoryPageSizeBytes, usage);
    m_totalAllocationSizeBytes          += pAllocator->getTotalSizeBytes();
    return pAllocator;
}


void VulkanAllocationManager::clear()
{
    for (auto it : m_resourceAllocators)
    {
        for (auto allocator : it.second)
        {
            allocator->clear();
        }
    }
}
} // Recluse