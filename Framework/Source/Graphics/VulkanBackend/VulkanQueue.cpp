//
#include "VulkanQueue.hpp"
#include "VulkanDevice.hpp"
#include "VulkanResource.hpp"

#include "VulkanCommandList.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

namespace Recluse {


VulkanQueue::~VulkanQueue()
{
    destroy();
}


ResultCode VulkanQueue::initialize(VulkanDevice* device, QueueFamily* pFamily, U32 queueFamilyIndex, U32 queueIndex)
{
    vkGetDeviceQueue(device->get(), queueFamilyIndex, queueIndex, &m_queue);

    m_pDevice                   = device;
    m_pFamilyRef                = pFamily;

    VkFenceCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        
    vkCreateFence(m_pDevice->get(), &info, nullptr, &m_fence);
    vkCreateFence(m_pDevice->get(), &info, nullptr, &m_oneTimeOnlyFence);

    VkCommandPoolCreateInfo poolCreateInfo = { };
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    vkCreateCommandPool(device->get(), &poolCreateInfo, nullptr, &m_tempCommandPool);
    
    return RecluseResult_Ok;
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
    if (m_tempCommandPool)
    {
        vkDestroyCommandPool(m_pDevice->get(), m_tempCommandPool, nullptr);
        m_tempCommandPool = VK_NULL_HANDLE;
    }
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

    if (m_oneTimeOnlyFence) {
    
        vkDestroyFence(m_pDevice->get(), m_oneTimeOnlyFence, nullptr);
        m_oneTimeOnlyFence = VK_NULL_HANDLE;
    
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


void VulkanQueue::generateCopyResource(VkCommandBuffer cmdBuffer, GraphicsResource* dst, GraphicsResource* src)
{

    ResourceDimension dstDim = dst->castTo<VulkanResource>()->getDimension();
    ResourceDimension srcDim = src->castTo<VulkanResource>()->getDimension();
    
    R_ASSERT_FORMAT(dst->isInResourceState(ResourceState_CopyDestination), "Destination resource not in Copy Destination state!");
    R_ASSERT_FORMAT(src->isInResourceState(ResourceState_CopySource), "Source resource not in Copy Source State!");

    if (dstDim == ResourceDimension_Buffer) 
    { 
        VulkanBuffer* dstBuffer = static_cast<VulkanBuffer*>(dst);

        if (srcDim == ResourceDimension_Buffer) 
        {
            VulkanBuffer* srcBuffer = static_cast<VulkanBuffer*>(src);
            VkBufferCopy region     = { };
            region.size             = srcBuffer->getBufferSizeBytes();
            region.dstOffset        = 0;
            region.srcOffset        = 0;
            vkCmdCopyBuffer(cmdBuffer, srcBuffer->get(), dstBuffer->get(), 1, &region);

        } 
        else 
        {
            VulkanImage* srcImage                   = static_cast<VulkanImage*>(src);
            VkBufferImageCopy region                = { };
            VkImageSubresourceRange sub             = srcImage->makeSubresourceRange(srcImage->getCurrentResourceState());
            region.imageSubresource.aspectMask      = sub.aspectMask;
            region.imageSubresource.baseArrayLayer  = sub.baseArrayLayer;
            region.imageSubresource.layerCount      = sub.layerCount;
            region.imageSubresource.mipLevel        = sub.baseMipLevel;
            region.imageExtent.width                = srcImage->getWidth();
            region.imageExtent.height               = srcImage->getHeight();
            region.imageExtent.depth                = srcImage->getDepthOrArraySize();
            region.imageOffset.x                    = 0;
            region.imageOffset.y                    = 0;
            region.imageOffset.z                    = 0;

            region.bufferOffset                     = 0;
            region.bufferRowLength                  = 0;
            region.bufferImageHeight                = 0;
            vkCmdCopyImageToBuffer
                (
                    cmdBuffer, 
                    srcImage->get(), 
                    srcImage->getCurrentLayout(),
                    dstBuffer->get(), 
                    1, 
                    &region
                );
        }
    }
    else
    {
        // Destination is an image.
        VulkanImage* dstImage = dst->castTo<VulkanImage>();
        if (srcDim == ResourceDimension_Buffer)
        {
            VulkanBuffer* srcBuffer = src->castTo<VulkanBuffer>();
            VkBufferImageCopy region = { };
            // TODO:
            VkImageSubresourceRange sub             = dstImage->makeSubresourceRange(dstImage->getCurrentResourceState());
            region.imageSubresource.aspectMask      = sub.aspectMask;
            region.imageSubresource.baseArrayLayer  = sub.baseArrayLayer;
            region.imageSubresource.layerCount      = sub.layerCount;
            region.imageSubresource.mipLevel        = sub.baseMipLevel;
            region.imageExtent.width                = dstImage->getWidth();
            region.imageExtent.height               = dstImage->getHeight();
            region.imageExtent.depth                = dstImage->getDepthOrArraySize();
            region.imageOffset.x                    = 0;
            region.imageOffset.y                    = 0;
            region.imageOffset.z                    = 0;

            region.bufferOffset                     = 0;
            region.bufferRowLength                  = 0;
            region.bufferImageHeight                = 0;
            vkCmdCopyBufferToImage(cmdBuffer, srcBuffer->get(), dstImage->get(), dstImage->getCurrentLayout(), 1, &region);
        }
        else
        {
            // TODO(Garcia): we might want to expose offset and mip/layer offsetting 
            //               regions per image copy.
            VulkanImage* srcImage = src->castTo<VulkanImage>();
            VkImageSubresourceRange srcRange = srcImage->makeSubresourceRange(srcImage->getCurrentResourceState());
            VkImageSubresourceRange dstRange = dstImage->makeSubresourceRange(dstImage->getCurrentResourceState());
            VkImageCopy region = { };
            region.dstOffset.x                      = 0;
            region.dstOffset.y                      = 0;
            region.dstOffset.z                      = 0;
            region.srcOffset.x                      = 0;
            region.srcOffset.y                      = 0;
            region.srcOffset.z                      = 0;
            region.extent.depth                     = Math::minimum(srcImage->getDepthOrArraySize(), dstImage->getDepthOrArraySize());
            region.extent.width                     = Math::minimum(srcImage->getWidth(), dstImage->getWidth());
            region.extent.height                    = Math::minimum(srcImage->getHeight(), dstImage->getHeight());
            region.dstSubresource.aspectMask        = dstRange.aspectMask;
            region.dstSubresource.baseArrayLayer    = dstRange.baseArrayLayer;
            region.dstSubresource.layerCount        = dstRange.layerCount;
            region.dstSubresource.mipLevel          = dstRange.baseMipLevel;
            region.srcSubresource.aspectMask        = srcRange.aspectMask;
            region.srcSubresource.baseArrayLayer    = srcRange.baseArrayLayer;
            region.srcSubresource.layerCount        = srcRange.layerCount;
            region.srcSubresource.mipLevel          = srcRange.baseMipLevel;
            vkCmdCopyImage
                (
                    cmdBuffer, 
                    srcImage->get(), srcImage->getCurrentLayout(), 
                    dstImage->get(), dstImage->getCurrentLayout(), 
                    1, &region
                );
        }
    }
}

ResultCode VulkanQueue::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    // Create a one time only command list.
    VkDevice device             = m_pDevice->get();
    VkCommandBuffer cmdBuffer   = beginOneTimeCommandBuffer();
    generateCopyResource(cmdBuffer, dst, src);
    endAndSubmitOneTimeCommandBuffer(cmdBuffer);
    
    return RecluseResult_Ok;
}


VkCommandBuffer VulkanQueue::beginOneTimeCommandBuffer()
{
    // We have null temp pool, we can't allocate.
    if (m_tempCommandPool == VK_NULL_HANDLE)
    {
        return VK_NULL_HANDLE;
    }

    // Create a one time only command list.
    VkDevice device             = m_pDevice->get();
    VkCommandBuffer cmdBuffer   = VK_NULL_HANDLE;
    VkResult result             = VK_SUCCESS;

    VkCommandBufferAllocateInfo allocInfo   = { };
    allocInfo.commandBufferCount            = 1;
    allocInfo.commandPool                   = m_tempCommandPool;
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


ResultCode VulkanQueue::endAndSubmitOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkDevice device             = m_pDevice->get();
    if (!commandBuffer)
    {
        return RecluseResult_InvalidArgs;
    }
    
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submit = { };
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandBuffer;
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkQueueSubmit(m_queue, 1, &submit, m_oneTimeOnlyFence);
    vkWaitForFences(device, 1, &m_oneTimeOnlyFence, true, UINT64_MAX);
    vkResetFences(device, 1, &m_oneTimeOnlyFence);
    freeOneTimeOnlyCommandBuffer(commandBuffer);
    return RecluseResult_Ok;
}


ResultCode VulkanQueue::copyBufferRegions
    (
        GraphicsResource* dst, 
        GraphicsResource* src, 
        const CopyBufferRegion* pRegions, 
        U32 numRegions
    )
{
    VkBuffer dstBuf             = static_cast<VulkanBuffer*>(dst)->get();
    VkBuffer srcBuf             = static_cast<VulkanBuffer*>(src)->get();
    R_ASSERT_FORMAT(dst->isInResourceState(ResourceState_CopyDestination), "Resource is not in Copy Destination state prior to a copy!");
    R_ASSERT_FORMAT(src->isInResourceState(ResourceState_CopySource), "Resource is not in a Copy Source state prior to a copy!");
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

    endAndSubmitOneTimeCommandBuffer(cmdBuffer);
    return RecluseResult_Ok;
}


void VulkanQueue::freeOneTimeOnlyCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkDevice device = m_pDevice->get();
    vkFreeCommandBuffers(device, m_tempCommandPool, 1, &commandBuffer);
}
} // Recluse