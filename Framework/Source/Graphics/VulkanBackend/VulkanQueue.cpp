//
#include "VulkanQueue.hpp"
#include "VulkanDevice.hpp"
#include "VulkanResource.hpp"

#include "VulkanCommandList.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


VulkanQueue::~VulkanQueue()
{
    destroy();
}


ErrType VulkanQueue::initialize(VulkanDevice* device, QueueFamily* pFamily, U32 queueFamilyIndex, U32 queueIndex)
{
    vkGetDeviceQueue(device->get(), queueFamilyIndex, queueIndex, &m_queue);

    m_pDevice                   = device;
    m_pFamilyRef                = pFamily;

    VkFenceCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        
    vkCreateFence(m_pDevice->get(), &info, nullptr, &m_fence);
    
    return REC_RESULT_OK;
}


void VulkanQueue::wait()
{
    VkResult result = vkQueueWaitIdle(m_queue);
    
    if (result != VK_SUCCESS) 
    {
        R_WARN(R_CHANNEL_VULKAN, "Failed to wait for queue idle!");
    }
}


void VulkanQueue::destroy()
{
    if (m_queue) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan queue handle...");
        wait();
        m_queue = VK_NULL_HANDLE;
    }

    if (m_fence) {
    
        vkDestroyFence(m_pDevice->get(), m_fence, nullptr);
        m_fence = VK_NULL_HANDLE;
    
    }
}

/*
ErrType VulkanQueue::submit(const QueueSubmit* payload)
{
    // Check if we need to clear our some flushes.
    m_pDevice->flushAllMappedRanges();
    m_pDevice->invalidateAllMappedRanges();

    VkSubmitInfo submitIf = { };
    VkCommandBuffer pBuffers[8];
    
    for (U32 bufferCount = 0; bufferCount < payload->numCommandLists; ++bufferCount) 
    {
        VulkanCommandList* pVc      = static_cast<VulkanCommandList*>(payload->pCommandLists[bufferCount]);
        VkCommandBuffer cmdBuffer   = pVc->get();
        pBuffers[bufferCount]       = cmdBuffer;
    }

    submitIf.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitIf.commandBufferCount     = payload->numCommandLists;
    submitIf.pCommandBuffers        = pBuffers;
    submitIf.waitSemaphoreCount     = 0;
    submitIf.signalSemaphoreCount   = 0;
    submitIf.pWaitDstStageMask      = nullptr;

    vkQueueSubmit(m_queue, 1, &submitIf, VK_NULL_HANDLE);
    
    return REC_RESULT_OK;
}
*/

ErrType VulkanQueue::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    const GraphicsResourceDescription& dstDesc = dst->getDesc();
    const GraphicsResourceDescription& srcDesc = src->getDesc();

    ResourceDimension dstDim = dstDesc.dimension;
    ResourceDimension srcDim = srcDesc.dimension;

    // Create a one time only command list.
    VkDevice device             = m_pDevice->get();
    VkCommandBuffer cmdBuffer   = beginOneTimeCommandBuffer();

    if (dstDim == RESOURCE_DIMENSION_BUFFER) 
    { 
        VulkanBuffer* dstBuffer = static_cast<VulkanBuffer*>(dst);

        if (srcDim == RESOURCE_DIMENSION_BUFFER) 
        {
            VulkanBuffer* srcBuffer = static_cast<VulkanBuffer*>(src);
            VkBufferCopy region     = { };
            region.size             = dstDesc.width;
            region.dstOffset        = 0;
            region.srcOffset        = 0;
            vkCmdCopyBuffer(cmdBuffer, srcBuffer->get(), dstBuffer->get(), 1, &region);

        } 
        else 
        {
            VulkanImage* pSrcImage      = static_cast<VulkanImage*>(src);
            VkBufferImageCopy region    = { };
            // TODO:
            vkCmdCopyImageToBuffer
                (
                    cmdBuffer, 
                    pSrcImage->get(), 
                    pSrcImage->getCurrentLayout(),
                    dstBuffer->get(), 
                    1, 
                    &region
                );
        }
    } 

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submit = { };
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuffer;
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkQueueSubmit(m_queue, 1, &submit, m_fence);
    
    vkWaitForFences(device, 1, &m_fence, true, UINT64_MAX);
    vkResetFences(device, 1, &m_fence);
    
    return REC_RESULT_OK;
}


VkCommandBuffer VulkanQueue::beginOneTimeCommandBuffer()
{
    // Create a one time only command list.
    VkDevice device             = m_pDevice->get();
    VkCommandBuffer cmdBuffer   = VK_NULL_HANDLE;
    VkResult result             = VK_SUCCESS;

    VkCommandBufferAllocateInfo allocInfo   = { };
    allocInfo.commandBufferCount            = 1;
    allocInfo.commandPool                   = m_pFamilyRef->commandPools[m_pDevice->getCurrentBufferIndex()];
    allocInfo.sType                         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level                         = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);

    if (result != VK_SUCCESS) 
    {
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo begin  = { };
    begin.sType                     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags                     = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuffer, &begin);

    return cmdBuffer;
}


ErrType VulkanQueue::copyBufferRegions
    (
        GraphicsResource* dst, 
        GraphicsResource* src, 
        CopyBufferRegion* pRegions, 
        U32 numRegions
    )
{
    VkBuffer dstBuf             = static_cast<VulkanBuffer*>(dst)->get();
    VkBuffer srcBuf             = static_cast<VulkanBuffer*>(src)->get();
    VkDevice device             = m_pDevice->get();
    VkCommandBuffer cmdBuffer   = beginOneTimeCommandBuffer();

    std::vector<VkBufferCopy> bufferCopies(numRegions);
    
    for (U32 i = 0; i < numRegions; ++i) 
    {
        bufferCopies[i].srcOffset = (VkDeviceSize)pRegions[i].srcOffsetBytes;
        bufferCopies[i].dstOffset = (VkDeviceSize)pRegions[i].dstOffsetBytes;
        bufferCopies[i].size      = (VkDeviceSize)pRegions[i].szBytes;
    }

    vkCmdCopyBuffer(cmdBuffer, srcBuf, dstBuf, numRegions, bufferCopies.data());

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submit = { };
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuffer;
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkQueueSubmit(m_queue, 1, &submit, m_fence);
    
    vkWaitForFences(device, 1, &m_fence, true, UINT64_MAX);
    vkResetFences(device, 1, &m_fence);
    
    return REC_RESULT_OK;
}
} // Recluse