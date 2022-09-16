//
#include "VulkanDescriptorManager.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "Recluse/Messaging.hpp"

#include <array>
#include <math.h>


namespace Recluse {


const U32 VulkanDescriptorManager::kMaxSetsPerPool          = 1000;
const F32 VulkanDescriptorManager::kAllocationPerResource   = 1000.f;

void VulkanDescriptorManager::initialize(VulkanDevice* pDevice)
{
    R_ASSERT(pDevice != NULL);

    m_device = pDevice->get();
}


void VulkanDescriptorManager::destroy(VulkanDevice* pDevice)
{
    VkDevice device = pDevice->get();

    for (auto pool : m_availablePools) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan descriptor pool...");
        vkDestroyDescriptorPool(device, pool, nullptr);
    }

    for (auto pool : m_usedPools)
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan descriptor pool...");
        vkDestroyDescriptorPool(device, pool, nullptr);
    }
}


VkDescriptorPool VulkanDescriptorManager::createDescriptorPool(const DescriptorPoolSizeFactors& poolSize, U32 maxSets, VkDescriptorPoolCreateFlags flags)
{
    R_ASSERT(m_device != VK_NULL_HANDLE);

    VkResult result                 = VK_SUCCESS;
    VkDescriptorPoolCreateInfo ci   = { };
    std::array<VkDescriptorPoolSize, 6> poolSizes;
    VkDescriptorPool resultingPool  = VK_NULL_HANDLE;

    poolSizes[0].type               = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount    = static_cast<U32>(poolSize.numberSamplersFactor * kAllocationPerResource);
    poolSizes[1].type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[1].descriptorCount    = static_cast<U32>(poolSize.numberSampledImagesFactor * kAllocationPerResource);
    poolSizes[2].type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount    = static_cast<U32>(poolSize.numberStorageBuffersFactor * kAllocationPerResource);
    poolSizes[3].type               = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[3].descriptorCount    = static_cast<U32>(poolSize.numberStorageImagesFactor * kAllocationPerResource);
    poolSizes[4].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[4].descriptorCount    = static_cast<U32>(poolSize.numberUbosFactor * kAllocationPerResource);
    poolSizes[5].type               = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[5].descriptorCount    = static_cast<U32>(poolSize.numberInputAttachmentsFactor * kAllocationPerResource);

    ci.sType                        = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.pPoolSizes                   = poolSizes.data();
    ci.poolSizeCount                = (U32)poolSizes.size();
    ci.maxSets                      = maxSets;
    ci.flags                        = flags;

    result = vkCreateDescriptorPool(m_device, &ci, nullptr, &resultingPool);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan descriptor pool!");
    }

    return resultingPool;
}


VulkanDescriptorAllocation VulkanDescriptorManager::allocate(VkDescriptorSet* pSets, U32 numberSetsToAlloc, VkDescriptorSetLayout* layouts)
{
    VkResult result                             = VK_SUCCESS;
    VkDescriptorSetAllocateInfo allocateInfo    = { };
    VulkanDescriptorAllocation allocation       = { };
    Bool needReallocate                         = false;

    if (m_currentPool == VK_NULL_HANDLE)
    {
        m_currentPool = getPool();
    }

    allocateInfo.descriptorPool                 = m_currentPool;
    allocateInfo.descriptorSetCount             = numberSetsToAlloc;
    allocateInfo.pSetLayouts                    = layouts;
    allocateInfo.sType                          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    result = vkAllocateDescriptorSets(m_device, &allocateInfo, pSets);

    switch (result)
    {
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
        {
            needReallocate = true;
            break;
        }
        default:
        {
            needReallocate = false;
            break;
        }
    }


    if (needReallocate)
    {
        m_currentPool               = getPool();
        allocateInfo.descriptorPool = m_currentPool;

        result = vkAllocateDescriptorSets(m_device, &allocateInfo, pSets);

        if (result != VK_SUCCESS)
            return allocation;
    }

    allocation = VulkanDescriptorAllocation(m_currentPool, numberSetsToAlloc);

    return allocation;
}


VkDescriptorPool VulkanDescriptorManager::getPool()
{
    if (m_availablePools.size() > 0)
    {
        VkDescriptorPool pool = m_availablePools.back();
        m_availablePools.pop_back();
        return pool;
    }
    else
    {
        DescriptorPoolSizeFactors sizes;
        VkDescriptorPool newPool = createDescriptorPool(sizes, kMaxSetsPerPool, 0);
        m_usedPools.push_back(newPool);
        return m_usedPools.back();
    }
}


ErrType VulkanDescriptorManager::free(const VulkanDescriptorAllocation& allocation, VkDescriptorSet* pSets, U32 numberSetsToFree)
{
    VkResult result = VK_SUCCESS;

    if (allocation.isValid())
    {
        VkDescriptorPool pool = allocation.getPool();
        result = vkFreeDescriptorSets(m_device, pool, numberSetsToFree, pSets);
    }
    else
    {
        return R_RESULT_INVALID_ARGS;
    }

    return (result == VK_SUCCESS) ? R_RESULT_OK : R_RESULT_FAILED;
}


void VulkanDescriptorManager::resetPools()
{
    for (auto pool : m_usedPools)
    {
        vkResetDescriptorPool(m_device, pool, 0);
        m_availablePools.push_back(pool);
    }

    m_usedPools.clear();

    m_currentPool = VK_NULL_HANDLE;
}
} // Recluse