// 
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Types.hpp"

#include <vector>
#include <map>

namespace Recluse {

class VulkanDevice;

struct VulkanMemory 
{
    U32                     allocatorIndex;
    U32                     memoryTypeIndex;
    VkDeviceMemory          deviceMemory;
    VkDeviceSize            offsetBytes;
    VkDeviceSize            sizeBytes;
    void*                   baseAddr;
};


// Vulkan allocator manager, handles memory heaps provided by Vulkan API.
//
class VulkanPagedAllocator 
{
public:
    VulkanPagedAllocator()
        : m_allocator(nullptr)
    { }

    ~VulkanPagedAllocator() { }

    ErrType initialize
                (
                    VkDevice device,
                    Allocator* pAllocator,
                    U32 memoryTypeIndex,
                    VkDeviceSize memorySizeBytes,
                    ResourceMemoryUsage usage,
                    U32 allocationId
                ) 
    {
        m_allocator             = pAllocator;
        m_pool.sizeBytes        = memorySizeBytes;
        m_allocationId          = allocationId;
        if (!m_allocator) return RecluseResult_NullPtrExcept;

        VkMemoryAllocateInfo info   = { };
        VkResult result             = VK_SUCCESS;
        info.sType                  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.memoryTypeIndex        = memoryTypeIndex;
        info.allocationSize         = memorySizeBytes;
        
        result = vkAllocateMemory(device, &info, nullptr, &m_pool.memory);
        
        if (result != VK_SUCCESS)
        {
            return RecluseResult_Failed;
        }

        if (usage != ResourceMemoryUsage_GpuOnly)
        { 
            result = vkMapMemory(device, m_pool.memory, 0, memorySizeBytes, 0, &m_pool.basePtr);
            if (result != VK_SUCCESS)
            {
                vkFreeMemory(device, m_pool.memory, nullptr);
                return RecluseResult_Failed;
            }
        }

        // Since we only need to work with offsets relative to the address,
        // We don't need the real address.
        m_allocator->initialize(0ull, m_pool.sizeBytes);
        return RecluseResult_Ok; 
    }

    // Allocate memory from program managed heap. This will require 
    // using automated allocation structures. 
    // @param pOut         The final result of our allocated resource, which contains the memory address from which was allocated, and the size allocated.
    // @param requirements Resource requirements for the given buffer or image object.
    // @param granularityBytes The granularity page size of memory addresses within the vulkan hardware. This is intended to ensure linear resources
    //                          and non-linear resources, do not inadverdently occupy the same memory sub-regions, so as to cause aliasing.
    //                          Considering this, we won't have an issue if we only allocate either only linear resources, or only non-linear resources
    //                          within the same allocator.
    // 
    // @return ErrType The result of the allocation, whether successful or not. If successful, returns pOut with filled data about the allocation. A failure
    //                 will give out the reason for the failure, along with no data fill in pOut.
    ErrType     allocate
                    (
                        VulkanMemory* pOut, 
                        const VkMemoryRequirements& requirements, 
                        VkDeviceSize granularityBytes, 
                        VkImageTiling tiling = VK_IMAGE_TILING_LINEAR
                    );

    // Free up memory, handles throwing memory into the garbage.
    // Immediate free can be done if we know we are in the same frame index.
    ErrType     free(VulkanMemory* pOut);

    // Destroy this allocation manager from the inside!
    void        release(VkDevice device);

    // clear out the whole memory heap.
    void        clear();

    Bool        hasSpace(VkDeviceSize requestedSizeBytes) const;

    VkDeviceSize getBaseAddress() const { return (VkDeviceSize)m_pool.basePtr; }

    U64         getTotalSizeBytes() const { return m_allocator->getTotalSizeBytes(); }
    U64         getUsedSizeBytes() const { return m_allocator->getUsedSizeBytes(); }
    U32         getAllocationId() const { return m_allocationId; }
    
private:
    VulkanMemoryPool                        m_pool;
    SmartPtr<Allocator>                     m_allocator;
    U32                                     m_allocationId;
};


// Allocation manager manages vulkan native memory in pages.
// Each page will have it's own allocator handler that will allocate memory when requested.
// Should a page be full, this will ultimately allocate a new page for memory, until we require the maximum allowable memory 
// set by the application.
class VulkanAllocationManager
{
public:
    // 
    static VkDeviceSize kPerMemoryPageSizeBytes;
    typedef U32 MemoryTypeIndex;

    enum VulkanAllocUpdateFlag
    {
        VulkanAllocUpdateFlag_None                  = 0,
        VulkanAllocUpdateFlag_Update                = (1 << 0),
        VulkanAllocUpdateFlag_SetFrameIndex         = (1 << 1),
        VulkanAllocUpdateFlag_IncrementFrameIndex   = (1 << 2),
        VulkanAllocUpdateFlag_GarbageResize         = (1 << 3),
        VulkanAllocUpdateFlag_Clear                 = (1 << 4)
    };

    typedef U32 VulkanAllocUpdateFlags;

    struct UpdateConfig
    {
        VulkanAllocUpdateFlags flags;
        U32 frameIndex : 16;
        U32 garbageBufferCount : 16;
    };

    VulkanAllocationManager()
        : m_garbageIndex(0u) 
        , m_numObjectAllocations(0ull)
        , m_totalAllocationSizeBytes(0ull)
        , m_pDevice(nullptr)
        , m_bufferImageGranularityBytes(0ull)
    {
    }

    ErrType     initialize(VulkanDevice* pDevice);
    void        clear();
    ErrType     release();

    ErrType     allocateBuffer(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements);
    ErrType     allocateImage(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling);
    ErrType     free(VulkanMemory* pOut, Bool immediate = false);

    void        update(const UpdateConfig& config);
    void        setTotalMemory(const MemoryReserveDesc& desc) { }

    U64         getTotalAllocationSizeBytes() const { return m_totalAllocationSizeBytes; }    

private:
    // Empty garbage from last frame.
    void emptyGarbage(U32 index);
    // Get the next allocator available.
    VulkanPagedAllocator* getAllocator(ResourceMemoryUsage usage, MemoryTypeIndex memoryTypeIndex, VkDeviceSize sizeBytes, VkDeviceSize alignment);
    // Allocate a page of memory if required.
    VulkanPagedAllocator* allocateMemoryPage(MemoryTypeIndex memoryTypeIndex, ResourceMemoryUsage usage);
    ErrType allocate(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling = VK_IMAGE_TILING_LINEAR);

    std::map<MemoryTypeIndex, std::vector<SmartPtr<VulkanPagedAllocator>>>      m_resourceAllocators;
    std::map<MemoryTypeIndex, U64>                                              m_pagedMemoryTotalSizeBytes;
    U32                                                                         m_garbageIndex;
    U32                                                                         m_numObjectAllocations;
    std::vector<std::vector<VulkanMemory>>                                      m_frameGarbage;
    VulkanDevice*                                                               m_pDevice;
    U64                                                                         m_totalAllocationSizeBytes;
    MemoryReserveDesc                                                           m_maxDedicatedMemoryDesc;
    VkDeviceSize                                                                m_bufferImageGranularityBytes;
};
} // Recluse