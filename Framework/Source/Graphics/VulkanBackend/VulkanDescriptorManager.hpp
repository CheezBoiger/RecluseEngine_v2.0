//
#pragma once

#include "VulkanCommons.hpp"

#include <vector>


namespace Recluse {


class VulkanDevice;


struct DescriptorPoolSizeFactors
{
    U32 numberSamplersFactor            = 0.5f;
    U32 numberSampledImagesFactor       = 1.0f;
    U32 numberStorageBuffersFactor      = 1.0f;
    U32 numberStorageImagesFactor       = 1.0f;
    U32 numberUbosFactor                = 1.0f;
    U32 numberInputAttachmentsFactor    = 1.0f;
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


class VulkanDescriptorAllocation
{
public:
    VulkanDescriptorAllocation(VkDescriptorPool pool = VK_NULL_HANDLE, U32 numberAllocs = 0)
        : pool(pool)
        , numberAllocations(numberAllocs)
    {
    }

    VkDescriptorPool getPool() { return pool; }
    VkDescriptorPool getPool() const { return pool; }

    U32 getNumberAllocations() const { return numberAllocations; }

    Bool isValid() const { return (pool != VK_NULL_HANDLE); }

    void invalidate() { pool = VK_NULL_HANDLE; numberAllocations = 0; }

private:
    VkDescriptorPool pool;
    U32              numberAllocations;
};


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
    VulkanDescriptorAllocation  allocate(VkDescriptorSet* pSets, U32 numberSetsToAlloc, VkDescriptorSetLayout* layouts);

    // Optionally, we can free the descriptor sets. Keep in mind that we don't need to call this, as 
    // destroy(), or reset(), will automatically clean up our allocations.
    ErrType                     free(const VulkanDescriptorAllocation& allocation, VkDescriptorSet* pSets, U32 numberSetsToFree);

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