//
#include "VulkanCommandList.hpp"
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
#include "VulkanPipelineState.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType VulkanCommandList::initialize(VulkanDevice* pDevice, U32 queueFamilyIndex, 
    VkCommandPool* pools, U32 poolCount)
{
    ErrType result                                  = REC_RESULT_OK;
    VkDevice device                                 = pDevice->get();

    m_buffers.resize(poolCount);
    m_pools.resize(m_buffers.size());

    for (U32 j = 0; j < poolCount; ++j) {

        VkCommandPool pool                    = pools[j];
        VkCommandBufferAllocateInfo allocInfo = { };

        allocInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool         = pool;
        allocInfo.commandBufferCount  = 1;
        allocInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkResult r = vkAllocateCommandBuffers(device, &allocInfo, &m_buffers[j]);

        if (r != VK_SUCCESS) {

            R_ERR(R_CHANNEL_VULKAN, "Failed to allocate command buffer!");

        }
        
        m_pools[j] = pools[j];
    }

    m_pDevice = pDevice;

    return result;
}


void VulkanCommandList::destroy(VulkanDevice* pDevice)
{
    for (U32 i = 0; i < m_buffers.size(); ++i) {
        
        if (m_buffers[i] != VK_NULL_HANDLE) {
        
            vkFreeCommandBuffers(pDevice->get(), m_pools[i], 1, &m_buffers[i]);
            m_buffers[i] = VK_NULL_HANDLE;
        
        }
    
    }
}


void VulkanCommandList::begin()
{
    VkCommandBufferBeginInfo info = { };
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.pInheritanceInfo   = nullptr;
    U32 bufIndex            = m_pDevice->getCurrentBufferIndex();
    VkCommandBuffer buffer  = m_buffers[bufIndex];

    resetBinds();

    VkResult result = vkBeginCommandBuffer(buffer, &info);

    R_ASSERT(result == VK_SUCCESS);

    m_currentCmdBuffer = buffer;
}


void VulkanCommandList::end()
{
    if (m_boundRenderPass) {
        endRenderPass(m_currentCmdBuffer);
    }

    VkResult result = vkEndCommandBuffer(m_currentCmdBuffer);
    
    R_ASSERT(result == VK_SUCCESS);
}


VkCommandBuffer VulkanCommandList::get() const
{
    U32 bufIdx = m_pDevice->getCurrentBufferIndex();
    return m_buffers[bufIdx];
}


void VulkanCommandList::setRenderPass(RenderPass* pRenderPass)
{
    R_ASSERT(pRenderPass != NULL);

    if (pRenderPass == m_boundRenderPass) {
        return;
    }

    VulkanRenderPass* pVrp  = static_cast<VulkanRenderPass*>(pRenderPass);
    
    VkImageMemoryBarrier barriers[9];
    U32 barrierCount        = 0;

    // End current render pass if it doesn't match this one...
    if (m_boundRenderPass != pVrp) {

        endRenderPass(m_currentCmdBuffer);
        
    }

    for (U32 i = 0; i < pVrp->getNumRenderTargets(); ++i) {
        VulkanResourceView* pRenderTarget   = static_cast<VulkanResourceView*>(pVrp->getRenderTarget(i));        
        VulkanImage* pImage                 = static_cast<VulkanImage*>(pRenderTarget->getResource());
        VkImageSubresourceRange range       = pRenderTarget->getSubresourceRange();

        if (pImage->getCurrentLayout() != pRenderTarget->getExpectedLayout()) {
            barriers[barrierCount++] = pImage->transition(pRenderTarget->getExpectedLayout(), range);
        }

    }

    if (barrierCount > 0) {

        vkCmdPipelineBarrier(m_currentCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 
            nullptr, 0, nullptr, barrierCount, barriers); 

    }

    VkRenderPassBeginInfo beginInfo = { };
    beginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.framebuffer           = pVrp->getFbo();
    beginInfo.renderPass            = pVrp->get();
    beginInfo.clearValueCount       = 0;
    beginInfo.pClearValues          = nullptr;
    beginInfo.renderArea            = pVrp->getRenderArea();

    vkCmdBeginRenderPass(m_currentCmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    m_boundRenderPass = pVrp;
}


void VulkanCommandList::endRenderPass(VkCommandBuffer buffer)
{
    if (m_boundRenderPass) {

        vkCmdEndRenderPass(buffer);
    }   
}


void VulkanCommandList::resetBinds()
{
    m_boundRenderPass = VK_NULL_HANDLE;
}


void VulkanCommandList::clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect)
{
    R_ASSERT(m_boundRenderPass != NULL);

    VkClearAttachment attachment    = { };
    VkClearValue color              = { };
    VkClearRect clearRect           = { };

    color.color.float32[0] = clearColor[0];
    color.color.float32[1] = clearColor[1];
    color.color.float32[2] = clearColor[2];
    color.color.float32[3] = clearColor[3];

    clearRect.layerCount = 1;
    clearRect.baseArrayLayer = 0;
    clearRect.rect.extent = { (U32)rect.width, (U32)rect.height };
    clearRect.rect.offset = { (I32)rect.x, (I32)rect.y };

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.clearValue = color;
    attachment.colorAttachment = idx;
    vkCmdClearAttachments(m_currentCmdBuffer, 1, &attachment, 1, &clearRect);
}


void VulkanCommandList::setPipelineState(PipelineState* pPipelineState, BindType bindType)
{
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    SETBIND(bindType, bindPoint)

    VulkanPipelineState* pVps   = static_cast<VulkanPipelineState*>(pPipelineState);
    VkPipeline pipeline         = pVps->get();
    
    vkCmdBindPipeline(m_currentCmdBuffer, bindPoint, pipeline);
}


void VulkanCommandList::bindDescriptorSets(U32 count, DescriptorSet** pSets, BindType bindType)
{
    R_ASSERT(m_boundPipelineState != NULL);
    R_ASSERT(count < 8);

    // Let's say we can only bound 8 descriptor sets at a time...
    VkDescriptorSet descriptorSets[8];

    U32 numDescriptorSetsBound      = 0;
    VkPipelineBindPoint bindPoint   = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkPipelineLayout layout         = m_boundPipelineState->getLayout();

    SETBIND(bindType, bindPoint)


    while (numDescriptorSetsBound < count) {

        VulkanDescriptorSet* pSet                   = static_cast<VulkanDescriptorSet*>(pSets[numDescriptorSetsBound]);
        descriptorSets[numDescriptorSetsBound++]    = pSet->get();

    }

    vkCmdBindDescriptorSets(m_currentCmdBuffer, bindPoint, layout, 0, numDescriptorSetsBound,
        descriptorSets, 0, nullptr);
}
} // Recluse