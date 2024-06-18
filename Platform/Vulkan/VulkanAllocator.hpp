// 
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Types.hpp"

#include <vector>
#include <map>

namespace Recluse {
namespace Vulkan {

class VulkanDevice;

struct VulkanMemory 
{
    U32                     allocatorIndex;     // The allocator index id that this memory object was allocated from.
    U32                     memoryTypeIndex;    // The memory type index that associates this memory object.
    // The device memory that this memory comes from. 
    // THIS IS NOT THE STARTING ADDRESS for this object, it is instead the address of 
    // the heap it came from.
    VkDeviceMemory          deviceMemory;
    // The offset bytes, which provides the starting address of this memory object.       
    VkDeviceSize            offsetBytes;
    // The actual size of this memory object that was allocated.
    VkDeviceSize            sizeBytes;
    // The base address representation of this memory block.
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

    // Initializes the allocator, which will handle one-time heap allocation. 
    ResultCode initialize(VkDevice device,
                          Allocator* pAllocator,
                          U32 memoryTypeIndex,
                          VkDeviceSize memorySizeBytes,
                          ResourceMemoryUsage usage,
                          U32 allocationId) 
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
        // We don't need to allocate based on the real address. The offset makes it easier to determine how far 
        // from the base address that our allocated object resides in, without needing to do extra calculations.
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
    ResultCode   allocate(VulkanMemory* pOut, 
                          const VkMemoryRequirements& requirements, 
                          VkDeviceSize granularityBytes, 
                          VkImageTiling tiling = VK_IMAGE_TILING_LINEAR);

    // Free up memory, handles throwing memory into the garbage.
    // Immediate free can be done if we know we are in the same frame index.
    ResultCode   free(VulkanMemory* pOut);

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

    // The given config flags to be used to control the behavior update for this manager.
    enum Flag
    {
        Flag_None                  = 0,
        Flag_Update                = (1 << 0),
        Flag_SetFrameIndex         = (1 << 1),
        Flag_IncrementFrameIndex   = (1 << 2),
        Flag_GarbageResize         = (1 << 3),
        Flag_Clear                 = (1 << 4)
    };

    typedef U32 Flags;

    // Provides update information and configuration to the update() call of this manager.
    struct UpdateConfig
    {
        Flags flags;
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

    // Initializes the manager with the given vulkan logical device. Be sure to call this first!
    ResultCode              initialize(VulkanDevice* pDevice);
    void                    clear();
    // Properly frees and releases all handles associated with this manager. Should be called after all resources are used, and no frames are inflight!
    ResultCode              release();

    // Allocates a requested buffer object.
    ResultCode              allocateBuffer(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements);
    // Allocates a requested image object.
    ResultCode              allocateImage(VulkanMemory* pOut, ResourceMemoryUsage usage, const VkMemoryRequirements& requirements, VkImageTiling tiling);
    // Frees the allocated vulkan memory. Unless immediate=true, this won't immediately free the vulkan object. Instead, it will be queued and freed the 
    // next time that same context frame it was called at, returns. This is to prevent conflicts with the freed memory object, should it be inflight, we ensure 
    // that the object can be safely freed from then. If this is not a concern, then give immediate=true to quickly free that object.
    ResultCode              free(VulkanMemory* pOut, Bool immediate = false);
    // Must be called, updates the manager for the given context frame. config allows controlling how to manage the update.
    // \param config The provided config that controls the update behavior for this manager.
    void                    update(const UpdateConfig& config);
    // Sets the total memory usage for this manager.
    void                    setTotalMemory(const MemoryReserveDescription& desc) { }
    U64                     getTotalAllocationSizeBytes() const { return m_totalAllocationSizeBytes; }    

private:
    // Empty garbage from last frame.
    void                    emptyGarbage(U32 index);
    // Get the next allocator available.
    VulkanPagedAllocator*   getAllocator(ResourceMemoryUsage usage, MemoryTypeIndex memoryTypeIndex, VkDeviceSize sizeBytes, VkDeviceSize alignment);
    // Allocate a page of memory if required.
    VulkanPagedAllocator*   allocateMemoryPage(MemoryTypeIndex memoryTypeIndex, ResourceMemoryUsage usage, VkDeviceSize pageSizeBytes);
    // Performs allocation of a resources with the provided requirements.
    // \param pOut output result of vulkan.
    // \param usage The memory usage that is requested.
    // \param requirements 
    // \param tiling Image tiling format.
    // 
    // \return The result of the allocation. RecluseResult_Ok if the allocation is a success. Any other result is a failure.
    ResultCode              allocate(VulkanMemory* pOut, 
                                     ResourceMemoryUsage usage,
                                     const VkMemoryRequirements& requirements,
                                     VkImageTiling tiling = VK_IMAGE_TILING_LINEAR);

    std::map<MemoryTypeIndex, std::vector<SmartPtr<VulkanPagedAllocator>>>      m_resourceAllocators;
    std::map<MemoryTypeIndex, U64>                                              m_pagedMemoryTotalSizeBytes;
    U32                                                                         m_garbageIndex;
    U32                                                                         m_numObjectAllocations;
    std::vector<std::vector<VulkanMemory>>                                      m_frameGarbage;
    VulkanDevice*                                                               m_pDevice;
    U64                                                                         m_totalAllocationSizeBytes;
    MemoryReserveDescription                                                    m_maxDedicatedMemoryDesc;
    VkDeviceSize                                                                m_bufferImageGranularityBytes;
    CriticalSection                                                             m_allocationCs;
};
} // Vulkan
} // Recluse