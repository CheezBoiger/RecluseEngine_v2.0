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


void VulkanCommandList::beginCommandList(U32 idx)
{
    VkCommandBufferBeginInfo info = { };
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.pInheritanceInfo   = nullptr;
    U32 bufIndex            = idx;
    VkCommandBuffer buffer  = m_buffers[bufIndex];

    resetBinds();

    VkResult result = vkBeginCommandBuffer(buffer, &info);

    R_ASSERT(result == VK_SUCCESS);
}


void VulkanCommandList::begin()
{
    U32 bufferIdx       = m_pDevice->getCurrentBufferIndex();
    m_currentCmdBuffer  = m_buffers[bufferIdx];
    m_currentIdx        = bufferIdx;

    beginCommandList(bufferIdx);

    m_status = COMMAND_LIST_RECORDING;
}


void VulkanCommandList::end()
{
    if (m_boundRenderPass) {
        endRenderPass(m_currentCmdBuffer);
    }

    endCommandList(m_currentIdx);

    m_status = COMMAND_LIST_READY;
}


void VulkanCommandList::endCommandList(U32 idx)
{
    VkCommandBuffer cmdbuffer = m_buffers[idx];

    VkResult result = vkEndCommandBuffer(cmdbuffer);
    
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

    // End current render pass if it doesn't match this one...
    if (m_boundRenderPass != pVrp) {

        endRenderPass(m_currentCmdBuffer);
        
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
        m_boundRenderPass = nullptr;

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

    m_boundPipelineState        = pVps;
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


void VulkanCommandList::bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets)
{
    VkBuffer vertexBuffers[8];
    
    for (U32 i = 0; i < numBuffers; ++i) {
    
        VulkanBuffer* pVb   = static_cast<VulkanBuffer*>(ppVertexBuffers[i]);
        vertexBuffers[i]    = pVb->get();
    
    }

    vkCmdBindVertexBuffers(m_currentCmdBuffer, 0, numBuffers, vertexBuffers, pOffsets);
}


void VulkanCommandList::drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance)
{
    vkCmdDraw(m_currentCmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}


void VulkanCommandList::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    const GraphicsResourceDescription& dstDesc = dst->getDesc();
    const GraphicsResourceDescription& srcDesc = src->getDesc();

    ResourceDimension dstDim = dstDesc.dimension;
    ResourceDimension srcDim = srcDesc.dimension;

    if (dstDim == RESOURCE_DIMENSION_BUFFER) { 
        VulkanBuffer* dstBuffer = static_cast<VulkanBuffer*>(dst);

        if (srcDim == RESOURCE_DIMENSION_BUFFER) {

            VulkanBuffer* srcBuffer = static_cast<VulkanBuffer*>(src);
            VkBufferCopy region     = { };
            region.size             = dstDesc.width;
            region.dstOffset        = 0;
            region.srcOffset        = 0;
            vkCmdCopyBuffer(m_currentCmdBuffer, srcBuffer->get(), dstBuffer->get(), 1, &region);

        } else {

            VulkanImage* pSrcImage      = static_cast<VulkanImage*>(src);
            VkBufferImageCopy region    = { };
            // TODO:
            vkCmdCopyImageToBuffer(m_currentCmdBuffer, pSrcImage->get(), pSrcImage->getCurrentLayout(),
                dstBuffer->get(), 1, &region);

        }

    } 
}


void VulkanCommandList::setViewports(U32 numViewports, Viewport* pViewports)
{
    VkViewport viewports[8];
    for (U32 i = 0; i < numViewports; ++i) {
    
        viewports[i].x          = pViewports[i].x;
        viewports[i].y          = pViewports[i].y;
        viewports[i].width      = pViewports[i].width;
        viewports[i].height     = pViewports[i].height;
        viewports[i].minDepth   = pViewports[i].minDepth;
        viewports[i].maxDepth   = pViewports[i].maxDepth;
    
    }

    vkCmdSetViewport(m_currentCmdBuffer, 0, numViewports, viewports);
}


void VulkanCommandList::setScissors(U32 numScissors, Rect* pRects) 
{
    VkRect2D scissors[8];
    for (U32 i = 0; i < numScissors; ++i) {

        scissors[i].extent.width    = (U32)pRects[i].width;
        scissors[i].extent.height   = (U32)pRects[i].height;
        scissors[i].offset.x        = (I32)pRects[i].x;
        scissors[i].offset.y        = (I32)pRects[i].y;    
    
    }

    vkCmdSetScissor(m_currentCmdBuffer, 0, numScissors, scissors);
}


void VulkanCommandList::dispatch(U32 x, U32 y, U32 z)
{
    endRenderPass(m_currentCmdBuffer);

    vkCmdDispatch(m_currentCmdBuffer, x, y, z);
}


void VulkanCommandList::transition(ResourceTransition* pTargets, U32 targetCounts)
{
    std::vector<VkImageMemoryBarrier> imgBarriers(targetCounts);
    U32 numBarriers = 0;

    for (U32 i = 0; i < targetCounts; ++i) {
        ResourceTransition& resTransition   = pTargets[i];
        VulkanImage* pVr                    = static_cast<VulkanImage*>(resTransition.pResource);
        VkImageSubresourceRange range       = { };
        
        range.baseArrayLayer                = resTransition.baseLayer;
        range.baseMipLevel                  = resTransition.baseMip;
        range.layerCount                    = resTransition.layers;
        range.levelCount                    = resTransition.mips;
        range.aspectMask                    = VK_IMAGE_ASPECT_COLOR_BIT;
        
        if (resTransition.dstState == RESOURCE_STATE_DEPTH_STENCIL_READONLY || 
            resTransition.dstState == RESOURCE_STATE_DEPTH_STENCIL_WRITE)
        {
            range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        imgBarriers[numBarriers++] = pVr->transition(resTransition.dstState, range);
    }

    if (numBarriers > 0) {

        // End any render pass that may not have been cleaned up.
        endRenderPass(m_currentCmdBuffer);

        vkCmdPipelineBarrier(m_currentCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 
            nullptr, 0, nullptr, numBarriers, imgBarriers.data()); 
    }
}

void VulkanCommandList::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type)
{
    VkBuffer buffer         = static_cast<VulkanBuffer*>(pIndexBuffer)->get();
    VkDeviceSize offset     = (VkDeviceSize)offsetBytes;
    VkIndexType indexType   = VK_INDEX_TYPE_UINT32;

    switch (indexType) {
        case INDEX_TYPE_UINT16: indexType = VK_INDEX_TYPE_UINT16; break;
        default: indexType = VK_INDEX_TYPE_UINT32; break;
    }

    vkCmdBindIndexBuffer(m_currentCmdBuffer, buffer, offset, indexType);
}

void VulkanCommandList::drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance)
{
    vkCmdDrawIndexed(m_currentCmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void VulkanCommandList::clearDepthStencil(F32 clearDepth, U8 clearStencil, const Rect& rect)
{
    R_ASSERT(m_boundRenderPass != NULL);

    VkClearRect clearRect                       = { };
    VkClearAttachment attachment                = { };
    
    attachment.aspectMask                       = VK_IMAGE_ASPECT_DEPTH_BIT;
    attachment.clearValue.depthStencil.depth    = clearDepth;
    attachment.clearValue.depthStencil.stencil  = clearStencil;
    attachment.colorAttachment                  = m_boundRenderPass->getNumRenderTargets(); // usually the last one.

    clearRect.baseArrayLayer    = 0;
    clearRect.layerCount        = 1;

    clearRect.rect.extent       = { (U32)rect.width, (U32)rect.height };
    clearRect.rect.offset       = { (I32)rect.x, (I32)rect.y };

    vkCmdClearAttachments(m_currentCmdBuffer, 1, &attachment, 1, &clearRect);
        
}
} // Recluse