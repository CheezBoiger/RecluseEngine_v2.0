//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Messaging.hpp"
#include <vector>


namespace Recluse {


class VulkanDevice;


struct DescriptorPoolSizeFactors
{
    F32 numberSamplersFactor            = 0.5f;
    F32 numberSampledImagesFactor       = 1.0f;
    F32 numberStorageBuffersFactor      = 1.0f;
    F32 numberStorageImagesFactor       = 1.0f;
    F32 numberUbosFactor                = 1.0f;
    F32 numberInputAttachmentsFactor    = 1.0f;
};


class VulkanDescriptorPool
{
public:




private:
    VkDescriptorPool m_pool;
    
    struct AllocatedResources
    {
        U32 allocatedSamplers;
        U32 allocatedSampledImages;
        U32 allocatedStorageBuffers;
        U32 allocatedStorageImages;
        U32 allocatedUbos;
        U32 allocatedInputAttachments;
    };

    AllocatedResources m_currentResources;
    DescriptorPoolSizeFactors m_maxResources;
};

// Meta information about the allocation done on Vulkan. This is used to store the 
// allocations that are done, along with find the pool that this allocation was done on.
class VulkanDescriptorAllocation
{
public:
    struct DescriptorSet
    {
        VkDescriptorSetLayout layout;
        VkDescriptorSet       set;
    };

    VulkanDescriptorAllocation
        (
            VkDescriptorPool pool = VK_NULL_HANDLE, 
            U32 numberAllocs = 0, 
            VkDescriptorSet* sets = nullptr, 
            VkDescriptorSetLayout* layouts = VK_NULL_HANDLE
        )
        : pool(pool)
        , size(numberAllocs)
    {
        // Do not attempt to allocate if we don't have anything to allocate.
        if (numberAllocs == 0)
            return;
        pSets = sets;
        pLayouts = layouts;
    }

    VulkanDescriptorAllocation(VulkanDescriptorAllocation&& alloc) noexcept
    {
        pool            = alloc.pool;
        pSets           = std::move(alloc.pSets);
        pLayouts        = std::move(alloc.pLayouts);
        size            = alloc.size;
        //alloc.pSets     = nullptr;
        //alloc.pLayouts  = nullptr;
    }

    VulkanDescriptorAllocation(const VulkanDescriptorAllocation& alloc)
    {
        pool        = alloc.pool;
        pSets       = alloc.pSets;
        pLayouts    = alloc.pLayouts;
        size        = alloc.size;
    }

    VulkanDescriptorAllocation& operator=(VulkanDescriptorAllocation&& alloc) noexcept
    {
        pool            = alloc.pool;
        pSets           = std::move(alloc.pSets);
        pLayouts        = std::move(alloc.pLayouts);
        size            = alloc.size;
        //alloc.pSets     = nullptr;
        //alloc.pLayouts  = nullptr;
        return (*this);
    }

    VulkanDescriptorAllocation& operator=(const VulkanDescriptorAllocation& alloc)
    {
        pool        = alloc.pool;
        pSets       = alloc.pSets;
        pLayouts    = alloc.pLayouts;
        size        = alloc.size;
    }

    ~VulkanDescriptorAllocation()
    {
        invalidate();
    }

    VkDescriptorPool getPool() { return pool; }
    VkDescriptorPool getPool() const { return pool; }

    U32 getNumberAllocations() const { return size; }

    Bool isValid() const { return (pool != VK_NULL_HANDLE); }

    void invalidate() 
    { 
        pool = VK_NULL_HANDLE;
        //pSets.release();
        //pLayouts.release();
        //pSets = nullptr;
        //pLayouts = nullptr;
        //pSets.clear();
        //pLayouts.clear();
        size = 0;
    }

    const DescriptorSet getDescriptorSet(U32 idx) const { return DescriptorSet{pLayouts[idx], pSets[idx]}; }
    const VkDescriptorSet* getNativeDescriptorSets() const { return pSets; }
    const VkDescriptorSetLayout* getNativeDescriptorSetLayout() const { return pLayouts; }

private:
    VkDescriptorPool                    pool;
    VkDescriptorSet*                    pSets;
    VkDescriptorSetLayout*              pLayouts;
    U32                                 size;
};


// Descriptor allocator instance is an object that handles allocation of descriptor sets from it's pool reserves.
// Serves as one instance that should be based on the number of instances to use by the engine.
class DescriptorAllocatorInstance 
{
public:
    static const U32 kMaxSetsPerPool;
    static const F32 kDescriptorChunkSize;
    static const F32 kDescriptorSamplerChunkSize;

    DescriptorAllocatorInstance() 
        : m_device(VK_NULL_HANDLE)
        , m_currentPool(VK_NULL_HANDLE) { }

    void                        initialize(VulkanDevice* pDevice, VkDescriptorPoolCreateFlags flags = 0);
    void                        release(VulkanDevice* pDevice);

    // Allocate the given number of descriptor sets. Normally the allocated sets should be allocated in the same descriptor pool.
    // Therefore special care should be taken when we allocate a number of sets, where partially some are allocated in a pool 
    // that is almost filled.
    VulkanDescriptorAllocation  allocate(U32 numberSetsToAlloc, VkDescriptorSetLayout* layouts);

    // Optionally, we can free the descriptor sets. Keep in mind that we don't need to call this, as 
    // destroy(), or reset(), will automatically clean up our allocations.
    ResultCode                     free(const VulkanDescriptorAllocation& allocation);

    // Reset the pools once we finish using. Will fully clear out all descriptor pools in this instance, which is a fast way to 
    // clear descriptor sets for the next frame. This will, however, require needing to re-allocate the needed descriptor sets for 
    // the next frame!
    void                        resetPools();

private:
    VkDescriptorPool            getPool();
    VkDescriptorPool            createDescriptorPool(const DescriptorPoolSizeFactors& poolSizes, U32 maxSets, VkDescriptorPoolCreateFlags flags);

    VkDescriptorPool                m_currentPool;
    std::vector<VkDescriptorPool>   m_availablePools;
    std::vector<VkDescriptorPool>   m_usedPools;
    VkDevice                        m_device;
    VkDescriptorPoolCreateFlags     m_flags;

    // TODO: For now we need to keep these here, but we want to make them global per frame, not once per descriptor allocator instance!
    CriticalSection                 m_allocatorCs;

    SmartPtr<Allocator>             m_descriptorSetAllocator;
    SmartPtr<Allocator>             m_descriptorSetLayoutAllocator;
    MemoryArena                     m_descriptorSetArena;
    MemoryArena                     m_descriptorSetLayoutArena;
};


// DescriptorAllocator handles buffered descriptor allocator instances, which must be worked like the memory allocator for vulkan.
// We need to keep track of all resources per buffer, freeing from the alignment of frames.
class DescriptorAllocator
{
public:
    const static U32                            kMaxReservedBufferInstances;
    // Initialize the descriptor allocator with the number of available buffers.
    void                                        initialize(VulkanDevice* pDevice, U32 bufferCount, VkDescriptorPoolCreateFlags flags = 0);
    // Release all descriptor allocator instances.
    void                                        release(VulkanDevice* pDevice);
    // Resize our buffered instances. Only if we need to resize instance, instead of releasing everything and reinitialize.
    void                                        resize(VulkanDevice* pDevice, U32 newBufferCount);
    
    // Get an allocator instance. Use the buffer index to find the right instance.
    DescriptorAllocatorInstance*                getInstance(U32 bufferIndex) { return &m_bufferedInstances[bufferIndex]; }
    U32                                         getBufferCount() const { return m_bufferedInstances.size(); }  

private:
    ResultCode                                  checkAndManageInstances(VulkanDevice* pDevice, U32 newbufferCount, VkDescriptorPoolCreateFlags flags);

    std::vector<DescriptorAllocatorInstance>    m_bufferedInstances;
    VkDescriptorPoolCreateFlags                 m_flags;
};
} // Recluse