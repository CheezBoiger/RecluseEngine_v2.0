//
#include "VulkanDescriptorManager.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Memory/LinearAllocator.hpp"

#include <array>
#include <math.h>


namespace Recluse {
namespace Vulkan {

const U32 DescriptorAllocatorInstance::kMaxSetsPerPool              = 1024;
const F32 DescriptorAllocatorInstance::kDescriptorChunkSize         = 512.f;
const F32 DescriptorAllocatorInstance::kDescriptorSamplerChunkSize  = 64.f;
const U32 DescriptorAllocator::kMaxReservedBufferInstances          = 16;

void DescriptorAllocatorInstance::initialize(VulkanDevice* pDevice, VkDescriptorPoolCreateFlags flags)
{
    R_ASSERT(pDevice != NULL);

    m_descriptorSetArena.preAllocate(sizeof(VkDescriptorSet) * kMaxSetsPerPool);
    m_descriptorSetLayoutArena.preAllocate(sizeof(VkDescriptorSetLayout) * kMaxSetsPerPool);

    m_descriptorSetAllocator = new LinearAllocator();
    m_descriptorSetLayoutAllocator = new LinearAllocator();
    
    m_descriptorSetAllocator->initialize(m_descriptorSetArena.getBaseAddress(), m_descriptorSetArena.getTotalSizeBytes());
    m_descriptorSetLayoutAllocator->initialize(m_descriptorSetLayoutArena.getBaseAddress(), m_descriptorSetLayoutArena.getTotalSizeBytes());

    m_allocatorCs.initialize();
    m_device = pDevice->get();
    m_flags = flags;
}


void DescriptorAllocatorInstance::release(VulkanDevice* pDevice)
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

    m_descriptorSetArena.release();
    m_descriptorSetLayoutArena.release();

    m_descriptorSetAllocator.release();
    m_descriptorSetLayoutAllocator.release();

    m_allocatorCs.release();
}


VkDescriptorPool DescriptorAllocatorInstance::createDescriptorPool(const DescriptorPoolSizeFactors& poolSize, U32 maxSets, VkDescriptorPoolCreateFlags flags)
{
    R_ASSERT(m_device != VK_NULL_HANDLE);

    VkResult result                 = VK_SUCCESS;
    VkDescriptorPoolCreateInfo ci   = { };
    std::array<VkDescriptorPoolSize, 6> poolSizes;
    VkDescriptorPool resultingPool  = VK_NULL_HANDLE;

    poolSizes[0].type               = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount    = static_cast<U32>(poolSize.numberSamplersFactor * kDescriptorSamplerChunkSize);
    poolSizes[1].type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[1].descriptorCount    = static_cast<U32>(poolSize.numberSampledImagesFactor * kDescriptorChunkSize);
    poolSizes[2].type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount    = static_cast<U32>(poolSize.numberStorageBuffersFactor * kDescriptorChunkSize);
    poolSizes[3].type               = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[3].descriptorCount    = static_cast<U32>(poolSize.numberStorageImagesFactor * kDescriptorChunkSize);
    poolSizes[4].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[4].descriptorCount    = static_cast<U32>(poolSize.numberUbosFactor * kDescriptorChunkSize);
    poolSizes[5].type               = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[5].descriptorCount    = static_cast<U32>(poolSize.numberInputAttachmentsFactor * kDescriptorChunkSize);

    ci.sType                        = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.pPoolSizes                   = poolSizes.data();
    ci.poolSizeCount                = (U32)poolSizes.size();
    ci.maxSets                      = maxSets;
    ci.flags                        = flags;

    result = vkCreateDescriptorPool(m_device, &ci, nullptr, &resultingPool);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create vulkan descriptor pool!");
    }

    return resultingPool;
}


VulkanDescriptorAllocation DescriptorAllocatorInstance::allocate(U32 numberSetsToAlloc, VkDescriptorSetLayout* layouts)
{
    VkResult result                             = VK_SUCCESS;
    VkDescriptorSetAllocateInfo allocateInfo    = { };
    VulkanDescriptorAllocation allocation       = { };
    Bool needReallocate                         = false;
    VkDescriptorSet* pAllocatedSets             = nullptr;
    VkDescriptorSetLayout* pAllocatedLayouts     = nullptr;

    {
        ScopedCriticalSection sc(m_allocatorCs);

        pAllocatedSets = (VkDescriptorSet*)m_descriptorSetAllocator->allocate(sizeof(VkDescriptorSet) * numberSetsToAlloc, 0);
        pAllocatedLayouts = (VkDescriptorSetLayout*)m_descriptorSetLayoutAllocator->allocate(sizeof(VkDescriptorSetLayout) * numberSetsToAlloc, 0);;
    }

    const SizeT descriptorSetLayoutBytes = sizeof(VkDescriptorSetLayout) * numberSetsToAlloc;
    memcpy(pAllocatedLayouts, layouts, descriptorSetLayoutBytes);

    if (m_currentPool == VK_NULL_HANDLE)
    {
        m_currentPool = getPool();
    }

    allocateInfo.descriptorPool                 = m_currentPool;
    allocateInfo.descriptorSetCount             = numberSetsToAlloc;
    allocateInfo.pSetLayouts                    = layouts;
    allocateInfo.sType                          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    result = vkAllocateDescriptorSets(m_device, &allocateInfo, pAllocatedSets);

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

        result = vkAllocateDescriptorSets(m_device, &allocateInfo, pAllocatedSets);

        if (result != VK_SUCCESS)
            return allocation;
    }

    allocation = VulkanDescriptorAllocation(m_currentPool, numberSetsToAlloc, pAllocatedSets, pAllocatedLayouts);

    return allocation;
}


VkDescriptorPool DescriptorAllocatorInstance::getPool()
{
    VkDescriptorPool pool = VK_NULL_HANDLE;
    if (m_availablePools.size() > 0)
    {
        pool = m_availablePools.back();
        m_availablePools.pop_back();
    }
    else
    {
        DescriptorPoolSizeFactors sizes;
        pool = createDescriptorPool(sizes, kMaxSetsPerPool, m_flags);
    }
    m_usedPools.push_back(pool);
    return pool;
}


ResultCode DescriptorAllocatorInstance::free(const VulkanDescriptorAllocation& allocation)
{
    VkResult result = VK_SUCCESS;

    if (allocation.isValid())
    {
        VkDescriptorPool pool = allocation.getPool();
        result = vkFreeDescriptorSets(m_device, pool, allocation.getNumberAllocations(), allocation.getNativeDescriptorSets());
    }
    else
    {
        return RecluseResult_InvalidArgs;
    }

    return (result == VK_SUCCESS) ? RecluseResult_Ok : RecluseResult_Failed;
}


void DescriptorAllocatorInstance::resetPools()
{
    for (auto pool : m_usedPools)
    {
        vkResetDescriptorPool(m_device, pool, 0);
        m_availablePools.push_back(pool);
    }

    m_usedPools.clear();
    m_descriptorSetAllocator->reset();
    m_descriptorSetLayoutAllocator->reset();

    m_currentPool = VK_NULL_HANDLE;
}


ResultCode DescriptorAllocator::checkAndManageInstances(VulkanDevice* pDevice, U32 newBufferCount, VkDescriptorPoolCreateFlags flags)
{
    R_ASSERT_FORMAT(newBufferCount <= kMaxReservedBufferInstances, "newBufferCount is greater than the maximum reserved instances. Cancelling this resize.");

    if (newBufferCount > kMaxReservedBufferInstances)
        return RecluseResult_Failed;

    I32 bufferedInstanceCount = static_cast<I32>(m_bufferedInstances.size());
    if (newBufferCount > bufferedInstanceCount)
    {
        // TODO: We need to resize this properly, without losing our existing allocaters!
        // If the new count is greater than existing, then we need to allocate.
        for (U32 i = bufferedInstanceCount; i < newBufferCount; ++i)
        {
            m_bufferedInstances.push_back(DescriptorAllocatorInstance());
            DescriptorAllocatorInstance& instance = m_bufferedInstances[i];
            instance.initialize(pDevice);
        }
    }
    else if (newBufferCount < bufferedInstanceCount)
    {
        // we can probably get away with not resizing the actual array?
        // We are shorter then we must deallocate our excess allocators.
        I32 i;
        for (i = (bufferedInstanceCount - 1); i >= (I32)newBufferCount; i--)
        {
            DescriptorAllocatorInstance& instance = m_bufferedInstances[i];
            instance.resetPools();
            instance.release(pDevice);
            m_bufferedInstances.pop_back();
        }

    }

    // If we are the same, don't do anything.
    return RecluseResult_Ok;
}


void DescriptorAllocator::initialize(VulkanDevice* pDevice, U32 bufferCount, VkDescriptorPoolCreateFlags flags)
{
    // Reserve a max of 16 buffer instances. We should not have that many buffered resources.
    m_bufferedInstances.reserve(kMaxReservedBufferInstances);
    checkAndManageInstances(pDevice, bufferCount, flags);

    m_flags = flags;
}


void DescriptorAllocator::release(VulkanDevice* pDevice)
{
    checkAndManageInstances(pDevice, 0, m_flags);
}


void DescriptorAllocator::resize(VulkanDevice* pDevice, U32 newBufferCount)
{
    checkAndManageInstances(pDevice, newBufferCount, m_flags);
}


ResultCode VulkanQueryManager::initialize(VkDevice device, VkQueryType type, U32 maxQueryCount)
{
    R_ASSERT(device && maxQueryCount > 0);
    ResultCode result = RecluseResult_Failed;
    if (!m_query && maxQueryCount > 0)
    {
        VkQueryPoolCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        info.queryType = type;
        info.queryCount = maxQueryCount;
        info.pNext = nullptr;
        info.flags = 0; // Reserved for future use.

        if (type == VK_QUERY_TYPE_PIPELINE_STATISTICS)
        {
            // If the query is a pipeline statistics type, we need to fill this info out too.
            info.pipelineStatistics =   VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                                        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                                        VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                                        VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT | 
                                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT | 
                                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT | 
                                        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
                                        VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
        }

        VkResult vkr = vkCreateQueryPool(device, &info, nullptr, &m_query);
        if (vkr == VK_SUCCESS)
            result = RecluseResult_Ok;
        m_maxQueryCount = maxQueryCount;
        m_type = type;
    }
    return result;
}


VulkanQueryManager::VulkanQueryManager()
    : m_query(nullptr)
    , m_currentAvailableIndex(0)
    , m_maxQueryCount(0)
    , m_type(VK_QUERY_TYPE_OCCLUSION)
{
}


VulkanQueryManager::~VulkanQueryManager()
{
    //R_ASSERT_FORMAT(!m_query, "A query pool was not released before destruction!");
}


ResultCode VulkanQueryManager::release(VkDevice device)
{
    if (m_query)
        vkDestroyQueryPool(device, m_query, nullptr);
    m_query = nullptr;
    return RecluseResult_Ok;
}


ResultCode VulkanQueryManager::reset(VkDevice device)
{   
    R_ASSERT(device);
    m_currentAvailableIndex = 0;
    vkResetQueryPool(device, m_query, m_currentAvailableIndex, m_maxQueryCount);
    return RecluseResult_Ok;
}


ResultCode VulkanQueryManager::resetLegacy(VkCommandBuffer cmdBuffer)
{
    m_currentAvailableIndex = 0;
    vkCmdResetQueryPool(cmdBuffer, m_query, m_currentAvailableIndex, m_maxQueryCount);
    return RecluseResult_Ok;
}


VulkanQueryManager::Index VulkanQueryManager::beginQuery(VkCommandBuffer cmdBuffer)
{
    Index index = allocateIndex();
    if (index != GraphicsQuery::InvalidQuery)
    {
        switch (m_type)
        {
            // TODO(): Both vulkan and d3d12 have similar behavior with timestamp queries, we should make a special function case for them.
            case VK_QUERY_TYPE_TIMESTAMP: vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_query, index); break;
            case VK_QUERY_TYPE_OCCLUSION: vkCmdBeginQuery(cmdBuffer, m_query, index, 0); break;
            default: break;
        }
    }
    
    return index;
}


void VulkanQueryManager::endQuery(VkCommandBuffer cmdBuffer, Index query)
{
    switch (m_type)
    {
        case VK_QUERY_TYPE_TIMESTAMP: 
        {
            Index timestampNext = allocateIndex();
            R_ASSERT_FORMAT((timestampNext - 1) == query, "Timestamp queries should be next to each other!");
            vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_query, timestampNext);
            break;
        }
        case VK_QUERY_TYPE_OCCLUSION: vkCmdEndQuery(cmdBuffer, m_query, query); break;
        default: break;
    }
}


VulkanQueryManager::Index VulkanQueryManager::allocateIndex()
{
    Index index = GraphicsQuery::InvalidQuery;
    if ((m_currentAvailableIndex + 1) < m_maxQueryCount)
    {
        index = m_currentAvailableIndex;
        m_currentAvailableIndex += 1;
    }
    return index;
}
} // Vulkan
} // Recluse