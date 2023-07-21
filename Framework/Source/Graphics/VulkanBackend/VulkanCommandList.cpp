//
#include "VulkanCommandList.hpp"
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
#include "VulkanPipelineState.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ResultCode VulkanPrimaryCommandList::initialize
                                (
                                    VulkanContext* pContext, 
                                    U32 queueFamilyIndex, 
                                    const VkCommandPool* pools, 
                                    U32 poolCount
                                )
{
    ResultCode result                                  = RecluseResult_Ok;
    VkDevice device                                 = static_cast<VulkanDevice*>(pContext->getDevice())->get();

    m_buffers.resize(poolCount);

    // For each command pool, be sure to allocate a command buffer handle
    // that reference our primary. We need to have one command buffer
    // for each frame reference.
    for (U32 j = 0; j < poolCount; ++j) 
    {
        VkCommandPool pool                    = pools[j];
        VkCommandBufferAllocateInfo allocInfo = { };
        
        allocInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool         = pool;
        allocInfo.commandBufferCount  = 1;
        allocInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkResult r = vkAllocateCommandBuffers(device, &allocInfo, &m_buffers[j]);

        if (r != VK_SUCCESS) 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Failed to allocate command buffer!");
        }
    }

    m_queueFamilyIndex  = queueFamilyIndex;

    return result;
}


void VulkanPrimaryCommandList::release(VulkanContext* pContext)
{
    // Destroy all buffers, ensure they correspond to the proper 
    // command pool.
    VulkanDevice* pDevice                           = static_cast<VulkanDevice*>(pContext->getDevice());
    VkCommandPool* pools                            = pContext->getCommandPools();
    for (U32 i = 0; i < m_buffers.size(); ++i) 
    {    
        if (m_buffers[i] != VK_NULL_HANDLE) 
        {   
            vkFreeCommandBuffers(pDevice->get(), pools[i], 1, &m_buffers[i]);
            m_buffers[i] = VK_NULL_HANDLE;
        }
    }
}


void VulkanPrimaryCommandList::reset()
{
    // If we are allowed to reset command lists.
    //vkResetCommandBuffer()
    R_ASSERT_FORMAT(m_status != CommandList_Reset, "Command list was already reset!");
    m_status = CommandList_Reset;
}


void VulkanPrimaryCommandList::beginCommandList(U32 idx)
{
    VkCommandBufferBeginInfo info = { };
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.pInheritanceInfo   = nullptr;
    U32 bufIndex            = idx;
    VkCommandBuffer buffer  = m_buffers[bufIndex];

    VkResult result = vkBeginCommandBuffer(buffer, &info);

    R_ASSERT(result == VK_SUCCESS);
}


void VulkanPrimaryCommandList::begin()
{
    R_ASSERT_FORMAT(m_status != CommandList_Recording, "Primary command list already begun recording! Only call GraphicsContext->begin() to begin the primary command list!");
    const U32 bufferIdx = m_currentIdx;
    m_currentCmdBuffer  = m_buffers[bufferIdx];
    m_currentIdx        = bufferIdx;

    beginCommandList(bufferIdx);

    m_status = CommandList_Recording;
}


void VulkanPrimaryCommandList::end()
{
    R_ASSERT_FORMAT(m_status == CommandList_Recording, "Primary command list has already ended! Only call GraphicsContext->end() to end the primary command list!");
    endCommandList(m_currentIdx);
    m_status = CommandList_Ready;
}


void VulkanPrimaryCommandList::endCommandList(U32 idx)
{
    VkCommandBuffer cmdbuffer = m_buffers[idx];

    VkResult result = vkEndCommandBuffer(cmdbuffer);
    R_ASSERT(result == VK_SUCCESS);
}


VkCommandBuffer VulkanPrimaryCommandList::get() const
{
    const U32 bufIdx = m_currentIdx;
    return m_buffers[bufIdx];
}


void VulkanContext::setRenderPass(VulkanRenderPass* pPass)
{
    R_ASSERT_FORMAT(pPass != NULL, "Null renderpass was set prior to render pass binding call!");
    R_ASSERT_FORMAT(pPass->getNumRenderTargets() <= 8, "Render Pass contains more than %d rtvs! This is more than hardware specs.", 8);

    flushBarrierTransitions(m_primaryCommandList.get());

    // End current render pass if it doesn't match this one. And begin the new pass.
    if (m_boundRenderPass != pPass->get()) 
    {
        endRenderPass(m_primaryCommandList.get());   
        VkRenderPassBeginInfo beginInfo = { };
        beginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.framebuffer           = pPass->getFbo();
        beginInfo.renderPass            = pPass->get();
        beginInfo.clearValueCount       = 0;
        beginInfo.pClearValues          = nullptr;
        beginInfo.renderArea            = pPass->getRenderArea();

        vkCmdBeginRenderPass(m_primaryCommandList.get(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_boundRenderPass               = pPass->get();
    }
}


void VulkanContext::resetBinds()
{
    m_boundRenderPass = VK_NULL_HANDLE;
    // Make sure we have at least one context state (this is our primary context state.)
    m_contextStates.clear();
    m_contextStates.push_back({ });
    currentState().m_srvs.clear();
    currentState().m_uavs.clear();
    currentState().m_samplers.clear();

    memset(currentState().m_cbvs.data(), 0, currentState().m_cbvs.size() * sizeof(DescriptorSets::ConstantBuffer));

    m_resourceViewShaderAccessMap.clear();
    m_constantBufferShaderAccessMap.clear();
    m_samplerShaderAccessMap.clear();
    m_pipelineState.pipeline    = VK_NULL_HANDLE;
    m_newRenderPass             = nullptr;
    m_boundRenderPass           = VK_NULL_HANDLE;
    m_boundDescriptorSet        = VK_NULL_HANDLE;
}


void VulkanContext::clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect)
{
    setRenderPass(m_newRenderPass);

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
    vkCmdClearAttachments(m_primaryCommandList.get(), 1, &attachment, 1, &clearRect);
}


void VulkanContext::bindPipelineState(const VulkanDescriptorAllocation& set)
{
    currentState().m_pipelineStructure.state.descriptorLayout = set.getDescriptorSet(0).layout;
    flushBarrierTransitions(m_primaryCommandList.get());

    if (!currentState().isPipelineDirty())
    {
        return;
    }

    const Pipelines::PipelineState pipelineState = Pipelines::makePipeline(getNativeDevice(), currentState().m_pipelineStructure);
    VkPipelineBindPoint bindPoint   = pipelineState.bindPoint;
    VkPipeline pipeline             = pipelineState.pipeline;
    if (pipeline != m_pipelineState.pipeline)
    { 
        vkCmdBindPipeline(m_primaryCommandList.get(), bindPoint, pipeline);
        m_pipelineState = pipelineState;
    }

    if (m_newRenderPass)
    {
        setRenderPass(m_newRenderPass);
        m_boundRenderPass = m_newRenderPass->get();
    }
}


void VulkanContext::bindDescriptorSet(const VulkanDescriptorAllocation& set)
{
    R_ASSERT(m_pipelineState.pipeline != NULL);
    flushBarrierTransitions(m_primaryCommandList.get());
    const VulkanDescriptorAllocation::DescriptorSet descriptorSet = set.getDescriptorSet(0);
    VkPipelineLayout layout                 = Pipelines::makeLayout(getNativeDevice(), descriptorSet.layout);
    const VkPipelineBindPoint bindPoint     = m_pipelineState.bindPoint;
    const VkDescriptorSet vSet              = descriptorSet.set;

    vkCmdBindDescriptorSets
        (
            m_primaryCommandList.get(),
            bindPoint, 
            layout, 
            0, 
            1,
            &vSet, 
            0, 
            nullptr
        );
}


void VulkanContext::bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets)
{

    flushBarrierTransitions(m_primaryCommandList.get());
    // Max number of vertex buffers to bind at a time.
    static VkBuffer vertexBuffers[8];
    
    for (U32 i = 0; i < numBuffers; ++i) 
    {
        VulkanBuffer* pVb   = static_cast<VulkanBuffer*>(ppVertexBuffers[i]);
        vertexBuffers[i]    = pVb->get();
    }

    vkCmdBindVertexBuffers(m_primaryCommandList.get(), 0, numBuffers, vertexBuffers, pOffsets);
}


void VulkanContext::drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance)
{
    flushBarrierTransitions(m_primaryCommandList.get());
    
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
    
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    vkCmdDraw(m_primaryCommandList.get(), vertexCount, instanceCount, firstVertex, firstInstance);
}


void VulkanContext::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    flushBarrierTransitions(m_primaryCommandList.get());

    const GraphicsResourceDescription& dstDesc = dst->getDesc();
    const GraphicsResourceDescription& srcDesc = src->getDesc();

    ResourceDimension dstDim = dstDesc.dimension;
    ResourceDimension srcDim = srcDesc.dimension;

    if (dstDim == ResourceDimension_Buffer) 
    { 
        VulkanBuffer* dstBuffer = static_cast<VulkanBuffer*>(dst);

        if (srcDim == ResourceDimension_Buffer) 
        {
            VulkanBuffer* srcBuffer = static_cast<VulkanBuffer*>(src);
            VkBufferCopy region     = { };
            region.size             = dstDesc.width;
            region.dstOffset        = 0;
            region.srcOffset        = 0;
            vkCmdCopyBuffer(m_primaryCommandList.get(), srcBuffer->get(), dstBuffer->get(), 1, &region);
        } 
        else 
        {
            VulkanImage* pSrcImage      = static_cast<VulkanImage*>(src);
            VkBufferImageCopy region    = { };
            // TODO:
            vkCmdCopyImageToBuffer
                (
                    m_primaryCommandList.get(),
                    pSrcImage->get(), 
                    pSrcImage->getCurrentLayout(),
                    dstBuffer->get(), 
                    1, 
                    &region
                );
        }
    } 
}


void VulkanContext::setViewports(U32 numViewports, Viewport* pViewports)
{
    VkViewport viewports[8];
    for (U32 i = 0; i < numViewports; ++i) 
    {
        viewports[i].x          = pViewports[i].x;
        viewports[i].y          = pViewports[i].y;
        viewports[i].width      = pViewports[i].width;
        viewports[i].height     = pViewports[i].height;
        viewports[i].minDepth   = pViewports[i].minDepth;
        viewports[i].maxDepth   = pViewports[i].maxDepth;
    }

    vkCmdSetViewport(m_primaryCommandList.get(), 0, numViewports, viewports);
}


void VulkanContext::setScissors(U32 numScissors, Rect* pRects) 
{
    VkRect2D scissors[8];
    for (U32 i = 0; i < numScissors; ++i) 
    {
        scissors[i].extent.width    = (U32)pRects[i].width;
        scissors[i].extent.height   = (U32)pRects[i].height;
        scissors[i].offset.x        = (I32)pRects[i].x;
        scissors[i].offset.y        = (I32)pRects[i].y;    
    }

    vkCmdSetScissor(m_primaryCommandList.get(), 0, numScissors, scissors);
}


void VulkanContext::dispatch(U32 x, U32 y, U32 z)
{
    endRenderPass(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    currentState().proposeClean();
    flushBarrierTransitions(m_primaryCommandList.get());
    vkCmdDispatch(m_primaryCommandList.get(), x, y, z);
}


void VulkanContext::transition(GraphicsResource* pResource, ResourceState dstState)
{
    R_ASSERT(pResource != NULL);
    R_ASSERT(pResource->getApi() == GraphicsApi_Vulkan);
    // End any render pass that may not have been cleaned up.
    endRenderPass(m_primaryCommandList.get());

    // Ignore the transition state, if we are already in this state.
    if (pResource->isInResourceState(dstState))
        return;

    VulkanResource* pVulkanResource = static_cast<VulkanResource*>(pResource);

    if (pVulkanResource->isBuffer())
    {
        VulkanBuffer* pBuffer = static_cast<VulkanBuffer*>(pVulkanResource);
        pBuffer->get();

        VkBufferMemoryBarrier barrier = pBuffer->transition(dstState);
        m_bufferMemoryBarriers.push_back(barrier);
    }
    else
    {
        VulkanImage* pVr                                = static_cast<VulkanImage*>(pVulkanResource);
        const GraphicsResourceDescription& description  = pVr->getDesc();
        VkImageSubresourceRange range                   = { };
        
        range.baseArrayLayer                = 0;
        range.baseMipLevel                  = 0;
        range.layerCount                    = description.depthOrArraySize;
        range.levelCount                    = description.mipLevels;
        range.aspectMask                    = VK_IMAGE_ASPECT_COLOR_BIT;
        
        if 
            (
                dstState == ResourceState_DepthStencilReadOnly || 
                dstState == ResourceState_DepthStencilWrite
            )
        {
            range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier barrier = pVr->transition(dstState, range);
        m_imageMemoryBarriers.push_back(barrier);
    }
}

void VulkanContext::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type)
{
    flushBarrierTransitions(m_primaryCommandList.get());

    VkBuffer buffer         = static_cast<VulkanBuffer*>(pIndexBuffer)->get();
    VkDeviceSize offset     = (VkDeviceSize)offsetBytes;
    VkIndexType indexType   = VK_INDEX_TYPE_UINT32;

    switch (indexType) 
    {
        case IndexType_Unsigned16:  indexType = VK_INDEX_TYPE_UINT16; break;
        default:                    indexType = VK_INDEX_TYPE_UINT32; break;
    }

    vkCmdBindIndexBuffer(m_primaryCommandList.get(), buffer, offset, indexType);
}

void VulkanContext::drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance)
{
    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    currentState().proposeClean();
    vkCmdDrawIndexed(m_primaryCommandList.get(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void VulkanContext::clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect)
{
    R_ASSERT(m_boundRenderPass != NULL);
    
    flushBarrierTransitions(m_primaryCommandList.get());

    VkClearRect clearRect                       = { };
    VkClearAttachment attachment                = { };

    VkImageAspectFlags flags = VK_IMAGE_ASPECT_NONE;
    if (clearFlags & ClearFlag_Depth)
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (clearFlags & ClearFlag_Stencil)
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    
    attachment.aspectMask                       = flags;
    attachment.clearValue.depthStencil.depth    = clearDepth;
    attachment.clearValue.depthStencil.stencil  = clearStencil;
    attachment.colorAttachment                  = m_newRenderPass->getNumRenderTargets(); // usually the last one.

    clearRect.baseArrayLayer    = 0;
    clearRect.layerCount        = 1;

    clearRect.rect.extent       = { (U32)rect.width, (U32)rect.height };
    clearRect.rect.offset       = { (I32)rect.x, (I32)rect.y };

    vkCmdClearAttachments(m_primaryCommandList.get(), 1, &attachment, 1, &clearRect);
        
}


void VulkanContext::drawIndexedInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride)
{
    R_ASSERT(pParams != NULL);
    R_ASSERT(pParams->getApi() == GraphicsApi_Vulkan);

    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    currentState().proposeClean();
    VulkanResource* pResource = pParams->castTo<VulkanResource>();

    if (!pResource->isBuffer())
    {
        R_ERROR("Vulkan", "Can not submit an indirect draw call if the resource is not a buffer! Ignoring call...");
        return;
    }

    const VulkanBuffer* pBuffer = pResource->castTo<VulkanBuffer>();

    vkCmdDrawIndexedIndirect(m_primaryCommandList.get(), pBuffer->get(), offset, drawCount, stride);
}


void VulkanContext::drawInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride)
{
    R_ASSERT(pParams != NULL);
    R_ASSERT(pParams->getApi() == GraphicsApi_Vulkan);

    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure); 
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    VulkanResource* pResource = static_cast<VulkanResource*>(pParams);
    currentState().proposeClean();

    if (!pResource->isBuffer())
    {
        R_ERROR("Vulkan", "Can not submit an indirect draw call if the resource is not a buffer! Ignoring call...");
        return;
    }

    const VulkanBuffer* pBuffer = pResource->castTo<VulkanBuffer>();

    vkCmdDrawIndirect(m_primaryCommandList.get(), pBuffer->get(), offset, drawCount, stride); 
}


void VulkanContext::dispatchIndirect(GraphicsResource* pParams, U64 offset)
{
    R_ASSERT(pParams != NULL);
    R_ASSERT(pParams->getApi() == GraphicsApi_Vulkan);

    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
        bindPipelineState(set); 
        bindDescriptorSet(set);
    }
    VulkanResource* pResource = pParams->castTo<VulkanResource>();
    
    if (pResource->isBuffer())
    {
        VulkanBuffer* pBuffer = pResource->castTo<VulkanBuffer>();
        const VkBuffer buffer = pBuffer->get();
        vkCmdDispatchIndirect(m_primaryCommandList.get(), buffer, offset);
    }
    else
    {
        R_ERROR("Vulkan", "ERROR: Can not use an image resource as an indirect args resource! Skipping call...");
    }
}


void VulkanContext::flushBarrierTransitions(VkCommandBuffer cmdBuffer)
{
    if (!m_bufferMemoryBarriers.empty() || !m_imageMemoryBarriers.empty())
    { 
        vkCmdPipelineBarrier
            (
                cmdBuffer,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_DEPENDENCY_BY_REGION_BIT,
                0,
                nullptr,
                static_cast<uint32_t>(m_bufferMemoryBarriers.size()),
                m_bufferMemoryBarriers.data(),
                static_cast<uint32_t>(m_imageMemoryBarriers.size()),
                m_imageMemoryBarriers.data()
            );
        
        m_bufferMemoryBarriers.clear();
        m_imageMemoryBarriers.clear();
    }
}


void VulkanContext::bindRenderTargets(U32 count, GraphicsResourceView** ppResourceViews, GraphicsResourceView* pDepthStencil)
{
    // Obtain the given render pass for the following resources. If one is already available, don't set it again!
    VulkanRenderPass* pPass = RenderPasses::makeRenderPass(m_pDevice, count, ppResourceViews, pDepthStencil);
    currentState().m_pipelineStructure.state.graphics.numRenderTargets = count;
    currentState().m_pipelineStructure.state.graphics.renderPass = pPass->get();
    m_newRenderPass = pPass;
    currentState().markPipelineDirty();
}


void VulkanContext::copyBufferRegions
    (
        GraphicsResource* dst, 
        GraphicsResource* src, 
        const CopyBufferRegion* pRegions, 
        U32 numRegions
    )
{
    VkBuffer dstBuf             = dst->castTo<VulkanBuffer>()->get();
    VkBuffer srcBuf             = src->castTo<VulkanBuffer>()->get();
    std::vector<VkBufferCopy> bufferCopies(numRegions);
    
    for (U32 i = 0; i < numRegions; ++i) 
    {
        bufferCopies[i].srcOffset = (VkDeviceSize)pRegions[i].srcOffsetBytes;
        bufferCopies[i].dstOffset = (VkDeviceSize)pRegions[i].dstOffsetBytes;
        bufferCopies[i].size      = (VkDeviceSize)pRegions[i].szBytes;
    }

    vkCmdCopyBuffer(m_primaryCommandList.get(), srcBuf, dstBuf, numRegions, bufferCopies.data());
}


void VulkanContext::bindConstantBuffer(ShaderType type, U32 slot, GraphicsResource* pResource, U64 offsetBytes, U64 sizeBytes)
{
    R_ASSERT_FORMAT(currentState().m_cbvs.size() > slot, "Maximum of %d constant buffers may be bound simultaneously. Request slot %d is not allowed.", currentState().m_cbvs.size(), slot);
    R_ASSERT(pResource->getApi() == GraphicsApi_Vulkan);
    ShaderStageFlags shaderFlags = shaderTypeToShaderStageFlags(type);
    VulkanResource* pVulkanResource = pResource->castTo<VulkanResource>();
    R_ASSERT(pVulkanResource->isBuffer());
    VulkanBuffer* pBuffer           = pVulkanResource->castTo<VulkanBuffer>();
    m_constantBufferShaderAccessMap[pBuffer->getId()] |= shaderFlags;
    currentState().m_cbvs[slot] = { pBuffer, static_cast<VkDeviceSize>(offsetBytes), static_cast<VkDeviceSize>(sizeBytes) };

    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppConstantBuffers         = currentState().m_cbvs.data();
    currentState().m_boundDescriptorSetStructure.key.value.constantBuffers = currentState().m_cbvs.size();
    currentState().markResourcesDirty();
}


void VulkanContext::bindShaderResources(ShaderType type, U32 offset, U32 count, GraphicsResourceView** ppResourceViews)
{
    currentState().m_srvs.reserve(count);
    ShaderStageFlags shaderFlags = shaderTypeToShaderStageFlags(type);
    for (U32 i = 0; i < count; ++i)
    {
        VulkanResourceView* pVulkanResourceView = ppResourceViews[i]->castTo<VulkanResourceView>();
        m_resourceViewShaderAccessMap[pVulkanResourceView->getId()] |= shaderFlags;
        currentState().m_srvs.push_back(pVulkanResourceView);
    }
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags     |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppShaderResources             = currentState().m_srvs.data();
    currentState().m_boundDescriptorSetStructure.key.value.srvs                = currentState().m_srvs.size();
    currentState().markResourcesDirty();
}


void VulkanContext::bindUnorderedAccessViews(ShaderType type, U32 offset, U32 count, GraphicsResourceView** ppResourceViews)
{
    currentState().m_uavs.reserve(count);
    ShaderStageFlags shaderFlags = shaderTypeToShaderStageFlags(type);
    for (U32 i = 0; i < count; ++i)
    {
        VulkanResourceView* pVulkanResourceView = ppResourceViews[i]->castTo<VulkanResourceView>();
        m_resourceViewShaderAccessMap[pVulkanResourceView->getId()] |= shaderFlags;
        currentState().m_uavs.push_back(pVulkanResourceView);
    }
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags     |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppUnorderedAccesses           = currentState().m_uavs.data();
    currentState().m_boundDescriptorSetStructure.key.value.uavs                = currentState().m_uavs.size();
    currentState().markResourcesDirty();
}


void VulkanContext::bindSamplers(ShaderType type, U32 count, GraphicsSampler** ppSamplers)
{
    currentState().m_samplers.reserve(count);
    ShaderStageFlags shaderFlags = shaderTypeToShaderStageFlags(type);
    for (U32 i = 0; i < count; ++i)
    {
        VulkanSampler* pVulkanSampler = ppSamplers[i]->castTo<VulkanSampler>();
        m_samplerShaderAccessMap[pVulkanSampler->getId()] |= shaderFlags;
        currentState().m_samplers.push_back(pVulkanSampler);
    }
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppSamplers                = currentState().m_samplers.data();
    currentState().m_boundDescriptorSetStructure.key.value.samplers        = currentState().m_samplers.size();
    currentState().markResourcesDirty();
}


void VulkanContext::pushState(ContextFlags flags)
{
    ContextState state = { };
    state.m_pipelineStructure = { };
    state.m_boundDescriptorSetStructure = { };

    if (flags & ContextFlag_InheritPipelineState)
    {
        state = *m_contextStates.end();
    }    

    m_contextStates.push_back(state);
}


void VulkanContext::popState()
{
    R_ASSERT_FORMAT(!m_contextStates.empty(), "We can not have 0 context states. Must at least be one context state!");
    if (m_contextStates.size() > 1)
    {
        m_contextStates.pop_back();
    }
}
} // Recluse