//
#include "VulkanAdapter.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"
#include "VulkanQueue.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/BuddyAllocator.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

namespace Recluse {
namespace Vulkan {

VkDeviceSize VulkanAllocationManager::kPerMemoryPageSizeBytes = R_MB(64);

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


ResultCode VulkanPagedAllocator::allocate(VulkanMemory* pOut, const VkMemoryRequirements& requirements, VkDeviceSize granularityBytes, VkImageTiling tiling)
{
     R_ASSERT(m_allocator != NULL);

    ResultCode result          = RecluseResult_Ok;
    UPtr baseAddr               = m_allocator->getBaseAddr();

    // Obtain the max alignment between granularity and memory requirement.
    // If the previous block contains a non-linear resource while the current one is linear or vice versa 
    // then the alignment requirement is the max of the VkMemoryRequirements.alignment and the device's bufferImageGranularity. 
    // This also needs to be check for the end of the memory block.
    // https://stackoverflow.com/questions/45458918/vulkan-memory-alignment-requirements
    U16 alignment = static_cast<U16>(Math::maximum(requirements.alignment, granularityBytes));
    // Align the block size of the requested memory with the needed requirements.
    // From there, allocate the aligned requested size, with the bufferImage granularity, so as to ensure we align with this.
    VkDeviceSize neededSizeBytes = requirements.size + requirements.alignment;
    UPtr allocatedAddress   = m_allocator->allocate((U64)neededSizeBytes, alignment);
    result                  = m_allocator->getLastError();

    if (result == RecluseResult_OutOfMemory)
    {
        return result;
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to allocate memory!");

        return result;
    }

    // The last alignment will be done with granularity.
    pOut->sizeBytes         = (U64)neededSizeBytes;
    pOut->offsetBytes       = allocatedAddress;
    pOut->deviceMemory      = m_pool.memory;
    pOut->baseAddr          = m_pool.basePtr;
    return result;
}


ResultCode VulkanPagedAllocator::free(VulkanMemory* pOut)
{
    R_ASSERT(m_allocator != NULL);
    
    if (!pOut) 
    {
        return RecluseResult_NullPtrExcept;
    }

    m_allocator->free(pOut->offsetBytes);
    return m_allocator->getLastError();    
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


ResultCode VulkanAllocationManager::release()
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

    m_allocationCs.release();

    return RecluseResult_Ok;
}


void VulkanAllocationManager::emptyGarbage(U32 index)
{
    std::vector<VulkanMemory>& garbage = m_frameGarbage[index];
    ResultCode result;

    for (U32 i = 0; i < garbage.size(); ++i) 
    {
        result = RecluseResult_Ok;
        
        VulkanMemory& mrange                = garbage[i];
        Allocation alloc                    = { };
        VulkanPagedAllocator* allocator     = m_resourceAllocators[mrange.memoryTypeIndex][mrange.allocatorIndex];

        alloc.baseAddress                   = mrange.offsetBytes;
        alloc.sizeBytes                     = mrange.sizeBytes;

        result = allocator->free(&mrange);

        if (result != RecluseResult_Ok) 
        {
            const U64 baseAddress = reinterpret_cast<U64>(mrange.baseAddr);
            R_ERROR
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
            (config.flags & Flag_GarbageResize) 
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
        
    if (config.flags & Flag_SetFrameIndex) 
    {
        if (config.frameIndex >= garbageSize) 
        {
            R_ERROR
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
    else if (config.flags & Flag_IncrementFrameIndex) 
    {
        m_garbageIndex = (m_garbageIndex + 1) % garbageSize;
    }

    if (config.flags & Flag_Update)
    {
        // We clean up the resources that we know should already be done for this buffer index.
        emptyGarbage(m_garbageIndex);
    }
}

void VulkanPagedAllocator::clear()
{
    R_ASSERT(m_pool.memory != VK_NULL_HANDLE);
    R_ASSERT(m_allocator    != NULL);

    m_allocator->reset();
}


ResultCode VulkanAllocationManager::initialize(VulkanDevice* device)
{
    m_pDevice = device;
    VkPhysicalDeviceProperties properties = m_pDevice->getAdapter()->getProperties();
    m_bufferImageGranularityBytes = properties.limits.bufferImageGranularity;
    m_allocationCs.initialize();
    // One frame index.
    m_frameGarbage.resize(1);
    return RecluseResult_Ok;
}


VulkanPagedAllocator* VulkanAllocationManager::getAllocator(ResourceMemoryUsage usage, MemoryTypeIndex memoryTypeIndex, VkDeviceSize sizeBytes, VkDeviceSize alignment)
{
    VulkanPagedAllocator* pAllocator    = nullptr;

    auto it = m_resourceAllocators.find(memoryTypeIndex);
    if (it == m_resourceAllocators.end())
    {
        pAllocator = allocateMemoryPage(memoryTypeIndex, usage, align(sizeBytes, alignment));
    }
    else
    {
        for (U32 i = 0; i < m_resourceAllocators[memoryTypeIndex].size(); ++i)
        {
            VulkanPagedAllocator* potentialAllocator = m_resourceAllocators[memoryTypeIndex][i];
            U64 adjustedAlignment = Math::maximum(alignment, m_bufferImageGranularityBytes);
            if (potentialAllocator->hasSpace(align(sizeBytes, adjustedAlignment)))
            {
                pAllocator = potentialAllocator;
                break;
            }
        }
    }

    if (!pAllocator)
    {
        pAllocator = allocateMemoryPage(memoryTypeIndex, usage, align(sizeBytes, alignment));
    }
    
    return pAllocator;
}


ResultCode VulkanAllocationManager::allocate(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling)
{
    VulkanAdapter* pAdapter                 = m_pDevice->getAdapter();
    MemoryTypeIndex memoryTypeIndex         = pAdapter->findMemoryType(requirements.memoryTypeBits, usage);

    ScopedCriticalSection cs(m_allocationCs);

    VulkanPagedAllocator* pagedAllocator    = getAllocator(usage, memoryTypeIndex, requirements.size, requirements.alignment);
    ResultCode result                       = RecluseResult_Ok;

    if (!pagedAllocator)
    {
        return RecluseResult_OutOfMemory;
    }

    result = pagedAllocator->allocate(pOut, requirements, m_bufferImageGranularityBytes, tiling);

    // PagedAllocator is out of memory, let's attempt to create a new page and allocate from there.
    if (result == RecluseResult_OutOfMemory)
    {
        pagedAllocator = allocateMemoryPage(memoryTypeIndex, usage, align(requirements.size, requirements.alignment));
        result = pagedAllocator->allocate(pOut, requirements, m_bufferImageGranularityBytes, tiling);
    }

    if (result != RecluseResult_Ok)
    {
        return result;
    }

    pOut->memoryTypeIndex = memoryTypeIndex;
    pOut->allocatorIndex = pagedAllocator->getAllocationId();

    return result;
}


ResultCode VulkanAllocationManager::allocateBuffer(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements)
{
    return allocate(pOut, usage, requirements);
}


ResultCode VulkanAllocationManager::allocateImage(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling)
{
    return allocate(pOut, usage, requirements, tiling);
}


ResultCode VulkanAllocationManager::free(VulkanMemory* pOut, Bool immediate)
{
    if (!pOut)
    {
        return RecluseResult_NullPtrExcept;
    }

    ResultCode result                   = RecluseResult_Ok;
    if (!immediate)
    {
        std::vector<VulkanMemory>& garbageChute = m_frameGarbage[m_garbageIndex];
        garbageChute.push_back(*pOut);
    }
    else
    {
        VulkanMemory& mrange                = *pOut;
        Allocation alloc                    = { };
        VulkanPagedAllocator* allocator     = m_resourceAllocators[mrange.memoryTypeIndex][mrange.allocatorIndex];

        alloc.baseAddress                   = mrange.offsetBytes;
        alloc.sizeBytes                     = mrange.sizeBytes;

        result = allocator->free(&mrange);

        if (result != RecluseResult_Ok) 
        {
            const U64 baseAddress = reinterpret_cast<U64>(mrange.baseAddr);
            R_ERROR
                (
                    "Allocator", 
                    "Failed to free garbage addr=%llu, at base addr=%llu", 
                    (U64)baseAddress + mrange.offsetBytes,
                    (U64)baseAddress
                );
        }
    }
    return result;
}


VulkanPagedAllocator* VulkanAllocationManager::allocateMemoryPage(MemoryTypeIndex memoryTypeIndex, ResourceMemoryUsage usage, VkDeviceSize pageSizeBytes)
{
    VkDevice device                     = m_pDevice->get();
    
#if defined(RECLUSE_DEBUG)
    {
        const VkPhysicalDeviceProperties& properties = m_pDevice->getAdapter()->getProperties();
        U32 numAllowedAllocations = static_cast<U32>(properties.limits.maxMemoryAllocationCount);
        U32 numAllocations = static_cast<U32>(m_resourceAllocators.size());
        R_ASSERT_FORMAT((numAllocations + 1) < numAllowedAllocations, "Vulkan device allows maximum %d allocations. We are going beyond that!", numAllowedAllocations);
    }
#endif

   m_resourceAllocators[memoryTypeIndex].push_back(makeSmartPtr(new VulkanPagedAllocator()));
    VulkanPagedAllocator* pAllocator    = m_resourceAllocators[memoryTypeIndex].back();
    const U32 allocationId              = (m_resourceAllocators[memoryTypeIndex].size() - 1);
    ResultCode result = pAllocator->initialize(device, new BuddyAllocator(), memoryTypeIndex, Math::maximum(align(pageSizeBytes, m_bufferImageGranularityBytes), align(kPerMemoryPageSizeBytes, m_bufferImageGranularityBytes)), usage, allocationId);
    m_totalAllocationSizeBytes          += pAllocator->getTotalSizeBytes();
    R_ASSERT_FORMAT(result == RecluseResult_Ok, "Failed to create new allocator for vulkan!");
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
} // Vulkan
} // Recluse