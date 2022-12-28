//
#pragma once

#include "VulkanCommons.hpp"

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

    VulkanDescriptorAllocation(VkDescriptorPool pool = VK_NULL_HANDLE, U32 numberAllocs = 0, VkDescriptorSet* sets = nullptr, VkDescriptorSetLayout* layouts = VK_NULL_HANDLE)
        : pool(pool)
        , size(numberAllocs)
    {
        pSets = new VkDescriptorSet[numberAllocs];
        pLayouts = new VkDescriptorSetLayout[numberAllocs];
        for (U32 i = 0; i < numberAllocs; ++i)
        {
            pSets[i] = sets[i];
            pLayouts[i] = layouts[i];
        }
    }

    VulkanDescriptorAllocation(VulkanDescriptorAllocation&& alloc)
    {
        pool = alloc.pool;
        pSets = std::move(alloc.pSets);
        pLayouts = std::move(alloc.pLayouts);
        size = alloc.size;
        alloc.pSets = nullptr;
        alloc.pLayouts = nullptr;
    }

    VulkanDescriptorAllocation(const VulkanDescriptorAllocation& alloc)
    {
        pool = alloc.pool;
        pSets = alloc.pSets;
        pLayouts = alloc.pLayouts;
        size = alloc.size;
    }

    VulkanDescriptorAllocation& operator=(VulkanDescriptorAllocation&& alloc)
    {
        pool = alloc.pool;
        pSets = std::move(alloc.pSets);
        pLayouts = std::move(alloc.pLayouts);
        size = alloc.size;
        alloc.pSets = nullptr;
        alloc.pLayouts = nullptr;
        return (*this);
    }

    VulkanDescriptorAllocation& operator=(const VulkanDescriptorAllocation& alloc)
    {
        pool = alloc.pool;
        pSets = alloc.pSets;
        pLayouts = alloc.pLayouts;
        size = alloc.size;
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
        pSets.release();
        pLayouts.release();
        pSets = nullptr;
        pLayouts = nullptr;
        size = 0;
    }

    const DescriptorSet getDescriptorSet(U32 idx) const { return DescriptorSet{pLayouts[idx], pSets[idx]}; }
    const VkDescriptorSet* getNativeDescriptorSets() const { return pSets.raw(); }
    const VkDescriptorSetLayout* getNativeDescriptorSetLayout() const { return pLayouts.raw(); }

private:
    VkDescriptorPool                pool;
    SmartPtr<VkDescriptorSet>       pSets;
    SmartPtr<VkDescriptorSetLayout> pLayouts;
    U32                             size;
};


// 
class VulkanDescriptorManager 
{
public:
    static const U32 kMaxSetsPerPool;
    static const F32 kAllocationPerResource;

    VulkanDescriptorManager() 
        : m_device(VK_NULL_HANDLE)
        , m_currentPool(VK_NULL_HANDLE) { }

    void                        initialize(VulkanDevice* pDevice);
    void                        destroy(VulkanDevice* pDevice);

    // Allocate the given number of descriptor sets. Normally the allocated sets should be allocated in the same descriptor pool.
    // Therefore special care should be taken when we allocate a number of sets, where partially some are allocated in a pool 
    // that is almost filled.
    VulkanDescriptorAllocation  allocate(U32 numberSetsToAlloc, VkDescriptorSetLayout* layouts);

    // Optionally, we can free the descriptor sets. Keep in mind that we don't need to call this, as 
    // destroy(), or reset(), will automatically clean up our allocations.
    ErrType                     free(const VulkanDescriptorAllocation& allocation);

    // Reset the pools once we finish using.
    void                        resetPools();

private:
    VkDescriptorPool    getPool();

    VkDescriptorPool    createDescriptorPool(const DescriptorPoolSizeFactors& poolSizes, U32 maxSets, VkDescriptorPoolCreateFlags flags);

    VkDescriptorPool                m_currentPool;
    std::vector<VkDescriptorPool>   m_availablePools;
    std::vector<VkDescriptorPool>   m_usedPools;
    VkDevice                        m_device;
};
} // Recluse