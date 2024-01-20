//
#include "VulkanCommandList.hpp"
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
#include "VulkanQueue.hpp"
#include "VulkanPipelineState.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Vulkan {

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


void VulkanContext::setRenderPass(const VulkanRenderPass& renderPass)
{
    R_ASSERT_FORMAT(!renderPass.isNull(), "Render pass must not be null!");
    // End current render pass if it doesn't match this one. And begin the new pass.
    if (m_boundRenderPass != renderPass.get()) 
    {
        endRenderPass(m_primaryCommandList.get());   
        VkRenderPassBeginInfo beginInfo = { };
        beginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.framebuffer           = renderPass.getFbo();
        beginInfo.renderPass            = renderPass.get();
        beginInfo.clearValueCount       = 0;
        beginInfo.pClearValues          = nullptr;
        beginInfo.renderArea            = renderPass.getRenderArea();

        vkCmdBeginRenderPass(m_primaryCommandList.get(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_boundRenderPass               = renderPass.get();
    }
}


void VulkanContext::internalBindVertexBuffersAndIndexBuffer()
{
    if (currentState().m_dirtyFlags & ContextDirtyFlag_VertexBuffers)
    {
        const uint32_t numVertexBuffers = currentState().m_numBoundVBs;
        VkBuffer* buffers               = currentState().m_vertexBuffers.data();
        U64* offsets                    = currentState().m_vbOffsets.data();
        vkCmdBindVertexBuffers(m_primaryCommandList.get(), 0, numVertexBuffers, buffers, offsets);
    }

    if (currentState().m_dirtyFlags & ContextDirtyFlag_IndexBuffer)
    {
        VkBuffer indexBuffer    = currentState().m_indexBuffer;
        VkDeviceSize offset     = currentState().m_ibOffsetBytes;
        VkIndexType indexType   = currentState().m_ibType;
        vkCmdBindIndexBuffer(m_primaryCommandList.get(), indexBuffer, offset, indexType);
    }
}


void VulkanContext::resetBinds()
{
    // Make sure we have at least one context state (this is our primary context state.)
    m_contextStates.clear();
    m_contextStates.push_back({ });

    clearResourceBinds();

    memset(currentState().m_vertexBuffers.data(), 0, currentState().m_vertexBuffers.size() * sizeof(VkBuffer));
    memset(currentState().m_vbOffsets.data(), 0, currentState().m_vbOffsets.size() * sizeof(U64));

    currentState().m_indexBuffer                                            = nullptr;
    currentState().m_numBoundVBs                                            = 0;
    currentState().m_ibOffsetBytes                                          = 0;
    currentState().m_pipelineStructure.nullify();

    m_resourceViewShaderAccessMap.clear();
    m_constantBufferShaderAccessMap.clear();
    m_samplerShaderAccessMap.clear();
    m_pipelineState             = { };
    m_boundRenderPass           = VK_NULL_HANDLE;
    m_boundDescriptorSet        = VK_NULL_HANDLE;
    m_newRenderPass.nullify();
}


void VulkanContext::clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect)
{
    flushBarrierTransitions(m_primaryCommandList.get());
    setRenderPass(m_newRenderPass);
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
    vkCmdClearAttachments(m_primaryCommandList.get(), 1, &attachment, 1, &clearRect);
}


void VulkanContext::bindPipelineState(const VulkanDescriptorAllocation& set)
{
    currentState().m_pipelineStructure.state.descriptorLayout = set.getDescriptorSet(0).layout;

    if (!currentState().isPipelineDirty())
    {
        return;
    }

    const Pipelines::PipelineState pipelineState = Pipelines::makePipeline(getNativeDevice(), currentState().m_pipelineStructure);
    VkPipelineBindPoint bindPoint   = pipelineState.bindPoint;
    VkPipeline pipeline             = pipelineState.pipeline;
    if (pipeline != m_pipelineState.pipeline)
    { 
        // End render pass if we bind a compute pipeline.
        if (bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE)
        {
            endRenderPass(m_primaryCommandList.get());
        }
        else
        {
            if (!m_newRenderPass.isNull())
            {
                setRenderPass(m_newRenderPass);
                m_boundRenderPass = m_newRenderPass.get();
            }
        }
        vkCmdBindPipeline(m_primaryCommandList.get(), bindPoint, pipeline);
        m_pipelineState = pipelineState;
    }
}


void VulkanContext::bindDescriptorSet(const VulkanDescriptorAllocation& set)
{
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
    // Max number of vertex buffers to bind at a time.
    static VkBuffer vertexBuffers[8];
    Bool areDifferentBuffers = false;
    currentState().m_numBoundVBs = numBuffers;
    for (U32 i = 0; i < numBuffers; ++i) 
    {
        VulkanBuffer* pVb   = static_cast<VulkanBuffer*>(ppVertexBuffers[i]);
        VkBuffer buffer = pVb->get();
        if (currentState().m_vertexBuffers[i] != buffer || currentState().m_vbOffsets[i] != pOffsets[i])
        {
            currentState().m_vertexBuffers[i] = buffer;
            currentState().m_vbOffsets[i] = pOffsets[i];
            areDifferentBuffers = true;
        }
    }
    if (areDifferentBuffers)
    {
        currentState().setDirty(ContextDirtyFlag_VertexBuffers);
    }
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
    if (currentState().areVertexBuffersOrIndexBuffersDirty())
    {
        internalBindVertexBuffersAndIndexBuffer();
    }
    currentState().proposeClean();
    vkCmdDraw(m_primaryCommandList.get(), vertexCount, instanceCount, firstVertex, firstInstance);
}


void VulkanContext::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    R_ASSERT_FORMAT(dst->isInResourceState(ResourceState_CopyDestination), "Resource is not in Copy Destination state prior to a copy!");
    R_ASSERT_FORMAT(src->isInResourceState(ResourceState_CopySource), "Resource is not in a Copy Source state prior to a copy!");
    flushBarrierTransitions(m_primaryCommandList.get());
    VulkanQueue::generateCopyResource(m_primaryCommandList.get(), dst, src);
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
    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    currentState().proposeClean();
    vkCmdDispatch(m_primaryCommandList.get(), x, y, z);
}


void VulkanContext::transition(GraphicsResource* pResource, ResourceState dstState, U16 baseMip, U16 mipCount, U16 baseLayer, U16 layerCount)
{
    R_ASSERT(pResource != NULL);

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
        VkImageSubresourceRange range                   = pVr->makeSubresourceRange(dstState, baseMip, mipCount, baseLayer, layerCount);
        VkImageMemoryBarrier barrier = pVr->transition(dstState, range);
        m_imageMemoryBarriers.push_back(barrier);
    }
}

void VulkanContext::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type)
{
    VkBuffer buffer         = static_cast<VulkanBuffer*>(pIndexBuffer)->get();
    VkDeviceSize offset     = (VkDeviceSize)offsetBytes;
    VkIndexType indexType   = VK_INDEX_TYPE_UINT32;

    switch (indexType) 
    {
        case IndexType_Unsigned16:  indexType = VK_INDEX_TYPE_UINT16; break;
        default:                    indexType = VK_INDEX_TYPE_UINT32; break;
    }

    if (buffer != currentState().m_indexBuffer || offsetBytes != currentState().m_ibOffsetBytes || currentState().m_ibType != indexType)
    {
        currentState().m_indexBuffer = buffer;
        currentState().m_ibOffsetBytes = offsetBytes;
        currentState().m_ibType = indexType;
        currentState().setDirty(ContextDirtyFlag_IndexBuffer);
    }
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
    if (currentState().areVertexBuffersOrIndexBuffersDirty())
    {
        internalBindVertexBuffersAndIndexBuffer();
    }
    currentState().proposeClean();
    vkCmdDrawIndexed(m_primaryCommandList.get(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void VulkanContext::clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect)
{
    flushBarrierTransitions(m_primaryCommandList.get());
    setRenderPass(m_newRenderPass);
    R_ASSERT(m_boundRenderPass != NULL);
    
    VkClearRect clearRect                       = { };
    VkClearAttachment attachment                = { };

    VkImageAspectFlags flags = 0;
    if (clearFlags & ClearFlag_Depth)
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (clearFlags & ClearFlag_Stencil)
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    
    const U32 numRenderTargets                  = currentState().m_pipelineStructure.state.graphics.numRenderTargets;
    attachment.aspectMask                       = flags;
    attachment.clearValue.depthStencil.depth    = clearDepth;
    attachment.clearValue.depthStencil.stencil  = clearStencil;
    attachment.colorAttachment                  = numRenderTargets ? (numRenderTargets - 1) : 0; // usually the last one.

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
    if (currentState().areVertexBuffersOrIndexBuffersDirty())
    {
        internalBindVertexBuffersAndIndexBuffer();
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
    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure); 
        bindPipelineState(set);
        bindDescriptorSet(set);
    }
    if (currentState().areVertexBuffersOrIndexBuffersDirty())
    {
        internalBindVertexBuffersAndIndexBuffer();
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
    flushBarrierTransitions(m_primaryCommandList.get());
    if (currentState().areResourcesDirty() || currentState().isPipelineDirty())
    {
        const VulkanDescriptorAllocation& set = DescriptorSets::makeDescriptorSet(this, currentState().m_boundDescriptorSetStructure);
        bindPipelineState(set); 
        bindDescriptorSet(set);
    }
    currentState().proposeClean();
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
        endRenderPass(m_primaryCommandList.get());
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


void VulkanContext::bindRenderTargets(U32 count, ResourceViewId* ppResourceViews, ResourceViewId pDepthStencil)
{
    // Obtain the given render pass for the following resources. If one is already available, don't set it again!
    m_newRenderPass = RenderPasses::makeRenderPass(getNativeDevice(), count, ppResourceViews, pDepthStencil);
    currentState().m_pipelineStructure.state.graphics.numRenderTargets = count;
    currentState().m_pipelineStructure.state.graphics.renderPass = m_newRenderPass.get();
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
    R_ASSERT_FORMAT(dst->isInResourceState(ResourceState_CopyDestination), "Resource is not in Copy Destination state prior to a copy!");
    R_ASSERT_FORMAT(src->isInResourceState(ResourceState_CopySource), "Resource is not in a Copy Source state prior to a copy!");
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


VulkanContext::ContextState& VulkanContext::VulkanShaderProgramBinder::currentState()
{
    return m_pContext->currentState();
}


IShaderProgramBinder& VulkanContext::VulkanShaderProgramBinder::bindConstantBuffer(ShaderStageFlags type, U32 slot, GraphicsResource* pResource, U32 offsetBytes, U32 sizeBytes, void* data)
{
    VulkanContext* context = m_pContext;
    R_ASSERT_FORMAT(currentState().m_cbvs.size() > slot, "Maximum of %d constant buffers may be bound simultaneously. Request slot %d is not allowed.", currentState().m_cbvs.size(), slot);
    ShaderStageFlags shaderFlags = type;
    U32 binding = slot;
    if (reflectionCache) 
    {
        R_ASSERT(slot < reflectionCache->cbvs.size());
        binding = reflectionCache->cbvs[slot];
    }

    DescriptorSets::BufferView bufferView = { nullptr, offsetBytes, sizeBytes, binding };
    if (pResource)
    {
        VulkanResource* pVulkanResource                     = pResource->castTo<VulkanResource>();
        R_ASSERT(pVulkanResource->isBuffer());
        VulkanBuffer* pBuffer                               = pVulkanResource->castTo<VulkanBuffer>();
        context->m_constantBufferShaderAccessMap[pBuffer->getId()]   |= shaderFlags;
        bufferView.buffer                                   = pBuffer;

        // Copy any data we may want to have in this constant buffer bind. It is actually similar the the d3d12 version, so we might want to make this agnostic.
        if (data)
        {
            void* pData = nullptr;
            MapRange readRange;
            readRange.offsetBytes = offsetBytes;
            readRange.sizeBytes = sizeBytes;
            ResultCode result = pBuffer->map(&pData, &readRange);
            if (result == RecluseResult_Ok)
            {
                memcpy(pData, data, sizeBytes);
                pBuffer->unmap(&readRange);
            }
        }
    }
    currentState().m_cbvs[slot] = bufferView;
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppConstantBuffers         = currentState().m_cbvs.data();
    currentState().m_boundDescriptorSetStructure.key.value.constantBuffers = Math::maximum(currentState().m_boundDescriptorSetStructure.key.value.constantBuffers, static_cast<U16>(slot+1));
    currentState().markResourcesDirty();
    return (*this);
}


IShaderProgramBinder& VulkanContext::VulkanShaderProgramBinder::bindShaderResource(ShaderStageFlags type, U32 slot, ResourceViewId viewId)
{
    VulkanContext* context = m_pContext;
    R_ASSERT_FORMAT(currentState().m_srvs.size() > slot, "Maximum of %d shader resource views may be bound simulatenously. Request slot %d is not allowed.", currentState().m_srvs.size(), slot);
    ShaderStageFlags shaderFlags = type;
    VulkanResourceView* pVulkanResourceView = ResourceViews::obtainResourceView(context->getNativeDevice()->getDeviceId(), viewId);
    if (pVulkanResourceView)
        context->m_resourceViewShaderAccessMap[pVulkanResourceView->getId()] |= shaderFlags;
    U32 binding = slot;
    if (reflectionCache)
    {
        R_ASSERT(slot < reflectionCache->srvs.size());
        binding = reflectionCache->srvs[slot];
    }
    currentState().m_srvs[slot] = { pVulkanResourceView, binding };
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags     |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppShaderResources             = currentState().m_srvs.data();
    currentState().m_boundDescriptorSetStructure.key.value.srvs                = Math::maximum(currentState().m_boundDescriptorSetStructure.key.value.srvs, static_cast<U16>(slot+1));
    currentState().markResourcesDirty();
    return (*this);
}


IShaderProgramBinder& VulkanContext::VulkanShaderProgramBinder::bindUnorderedAccessView(ShaderStageFlags type, U32 slot, ResourceViewId view)
{
    VulkanContext* context = m_pContext;
    ShaderStageFlags shaderFlags = type;
    VulkanResourceView* pVulkanResourceView = ResourceViews::obtainResourceView(context->getNativeDevice()->getDeviceId(), view);
    if (pVulkanResourceView)
        context->m_resourceViewShaderAccessMap[pVulkanResourceView->getId()] |= shaderFlags;
    U32 binding = slot;
    if (reflectionCache)
    {
        R_ASSERT(slot < reflectionCache->uavs.size());
        binding = reflectionCache->uavs[slot];
    }
    currentState().m_uavs[slot] = { pVulkanResourceView, binding };
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags     |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppUnorderedAccesses           = currentState().m_uavs.data();
    currentState().m_boundDescriptorSetStructure.key.value.uavs                = Math::maximum(currentState().m_boundDescriptorSetStructure.key.value.uavs, static_cast<U16>(slot+1));
    currentState().markResourcesDirty();
    return (*this);
}


IShaderProgramBinder& VulkanContext::VulkanShaderProgramBinder::bindSampler(ShaderStageFlags type, U32 slot, GraphicsSampler* ppSamplers)
{
    VulkanContext* context = m_pContext;
    ShaderStageFlags shaderFlags = type;
    VulkanSampler* pVulkanSampler = nullptr; 
    if (ppSamplers)
    { 
        pVulkanSampler = ppSamplers->castTo<VulkanSampler>();
        context->m_samplerShaderAccessMap[pVulkanSampler->getId()] |= shaderFlags;
    }
    U32 binding = slot;
    if (reflectionCache)
    {
        R_ASSERT(slot < reflectionCache->samplers.size());
        binding = reflectionCache->samplers[slot];
    }
    currentState().m_samplers[slot] = { pVulkanSampler, binding };
    currentState().m_boundDescriptorSetStructure.key.value.shaderTypeFlags |= shaderFlags;
    currentState().m_boundDescriptorSetStructure.ppSamplers                = currentState().m_samplers.data();
    currentState().m_boundDescriptorSetStructure.key.value.samplers        = Math::maximum(currentState().m_boundDescriptorSetStructure.key.value.samplers, static_cast<U16>(slot+1));
    currentState().markResourcesDirty();
    return (*this);
}


void VulkanContext::clearResourceBinds()
{
    memset(currentState().m_cbvs.data(), 0, currentState().m_cbvs.size() * sizeof(DescriptorSets::BufferView));
    memset(currentState().m_srvs.data(), 0, currentState().m_srvs.size() * sizeof(VulkanResourceView*)); // This is ok, we are weak referencing.
    memset(currentState().m_uavs.data(), 0, currentState().m_uavs.size() * sizeof(VulkanResourceView*)); // Same, just weak references.
    memset(currentState().m_samplers.data(), 0, currentState().m_samplers.size() * sizeof(VulkanSampler*));

    currentState().m_boundDescriptorSetStructure.key.value.constantBuffers  = 0;
    currentState().m_boundDescriptorSetStructure.key.value.srvs             = 0;
    currentState().m_boundDescriptorSetStructure.key.value.uavs             = 0;
    currentState().m_boundDescriptorSetStructure.key.value.samplers         = 0;
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
} // Vulkan
} // Recluse