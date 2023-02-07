//
#include "VulkanAdapter.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

namespace Recluse {

VkDeviceSize VulkanAllocationManager::kPerMemoryPageSizeBytes = R_MB(8);

// Understanding buffer-image granularity is one of the pains of Vulkan.
// Here is a quick visual reference description by akeley98 who describes this well:
// https://www.reddit.com/r/vulkan/comments/krdrxi/how_to_allocate_with_bufferimagegranularity/
//
static B32 areMemoryResourcesOnSeparatePages
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


ErrType VulkanPagedAllocator::allocate(VulkanMemory* pOut, const VkMemoryRequirements& requirements, VkDeviceSize granularityBytes, VkImageTiling tiling)
{
     R_ASSERT(m_allocator != NULL);
    
    Allocation allocation   = { };
    ErrType result          = RecluseResult_Ok;
    PtrType baseAddr        = m_allocator->getBaseAddr();

    // Obtain the max alignment between granularity and memory requirement.
    // If the previous block contains a non-linear resource while the current one is linear or vice versa 
    // then the alignment requirement is the max of the VkMemoryRequirements.alignment and the device's bufferImageGranularity. 
    // This also needs to be check for the end of the memory block.
    // https://stackoverflow.com/questions/45458918/vulkan-memory-alignment-requirements
    U16 alignment = static_cast<U16>(Math::maximum(requirements.alignment, granularityBytes));
    // Align the block size of the requested memory with the needed requirements.
    // From there, allocate the aligned requested size, with the bufferImage granularity, so as to ensure we align with this.
    result = m_allocator->allocate(&allocation, (U64)requirements.size, alignment);
    
    if (result == RecluseResult_OutOfMemory)
    {
        return result;
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate memory!");

        return result;
    }

    // The last alignment will be done with granularity.
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
    //{
    //    // Push back a copy.
    //    std::vector<VulkanMemory>& garbageChute = m_frameGarbage[m_garbageIndex];
    //    garbageChute.push_back(*pOut);
    //}

    return m_allocator->free(&allocation);
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
    VkPhysicalDeviceProperties properties = m_pDevice->getAdapter()->getProperties();
    m_bufferImageGranularityBytes = properties.limits.bufferImageGranularity;
    return RecluseResult_Ok;
}


VulkanPagedAllocator* VulkanAllocationManager::getAllocator(const VkMemoryRequirements& requirements, ResourceMemoryUsage usage)
{
    VkDevice device                     = m_pDevice->get();
    VulkanAdapter* pAdapter             = m_pDevice->getAdapter();
    MemoryTypeIndex memoryTypeIndex     = pAdapter->findMemoryType(requirements.memoryTypeBits, usage);
    VulkanPagedAllocator* pAllocator    = nullptr;

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
            U64 alignment = Math::maximum(requirements.alignment, m_bufferImageGranularityBytes);
            if (potentialAllocator->hasSpace(align(requirements.size, alignment)))
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
    
    return pAllocator;
}


ErrType VulkanAllocationManager::allocateBuffer(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements)
{
    VulkanPagedAllocator* pAllocator = getAllocator(requirements, usage);
    return pAllocator->allocate(pOut, requirements, m_bufferImageGranularityBytes);
}


ErrType VulkanAllocationManager::allocateImage(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling)
{
    VulkanPagedAllocator* allocator = getAllocator(requirements, usage);
    return allocator->allocate(pOut, requirements, m_bufferImageGranularityBytes, tiling);
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