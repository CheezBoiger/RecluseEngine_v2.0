// 
#include "Recluse/Types.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanQueue.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
#include "VulkanCommandList.hpp"
#include "Recluse/Messaging.hpp"


namespace Recluse {


GraphicsResource* VulkanSwapchain::getFrame(U32 idx)
{
    return m_frameResources[idx];
}


GraphicsResourceView* VulkanSwapchain::getFrameView(U32 idx)
{
    return m_frameViews[idx];
}


ErrType VulkanSwapchain::build(VulkanDevice* pDevice)
{
    VkSwapchainCreateInfoKHR createInfo         = { };
    const SwapchainCreateDescription& pDesc     = getDesc();
    VkSurfaceKHR surface                        = pDevice->getSurface();
    VkResult result                             = VK_SUCCESS;
    VkFormat vkFormat                           = Vulkan::getVulkanFormat(pDesc.format);
    VkColorSpaceKHR colorSpace                  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    if (pDesc.renderWidth <= 0 || pDesc.renderHeight <= 0) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Can not define a RenderWidth or Height less than 1!");

        return RecluseResult_InvalidArgs;    
    }

    {
        VulkanAdapter* pAdapter = pDevice->getAdapter();
        std::vector<VkSurfaceFormatKHR> supportedFormats = pAdapter->getSurfaceFormats(surface);
        for (VkSurfaceFormatKHR& sF : supportedFormats) 
        {
            if (sF.format == vkFormat) 
            {
                colorSpace = sF.colorSpace;
                break;
            } 
            else 
            {
                vkFormat = VK_FORMAT_UNDEFINED;
            }
        }

        if (vkFormat == VK_FORMAT_UNDEFINED) 
        {
            // Use the default first.
            R_WARN
                (
                    R_CHANNEL_VULKAN, 
                    "Can not find the requested format (%s), Using default supported surface format (%s).", 
                    getResourceFormatString(pDesc.format), 
                    getResourceFormatString(Vulkan::getResourceFormat(supportedFormats[0].format))
                );
            R_ASSERT_MSG(!supportedFormats.empty(), "No supported formats were found for this physical device!");
            vkFormat    = supportedFormats[0].format;
            colorSpace  = supportedFormats[0].colorSpace;
        }
    }
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = colorSpace;
    createInfo.imageFormat      = vkFormat;
    createInfo.surface          = surface;
    createInfo.imageExtent      = { pDesc.renderWidth, pDesc.renderHeight };
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.minImageCount    = pDesc.desiredFrames;
    createInfo.imageArrayLayers = 1;
    createInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; 
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.queueFamilyIndexCount = 0;

    switch (pDesc.buffering) 
    {
        default:
        case FrameBuffering_Single: createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; break;
        case FrameBuffering_Double: createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; break;
        case FrameBuffering_Triple: createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; break;
    }
    
    result = vkCreateSwapchainKHR(pDevice->get(), &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) 
    {    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create swapchain!");

        return RecluseResult_Failed;
    }    

    R_DEBUG(R_CHANNEL_VULKAN, "Successfully created vulkan swapchain!");
    
    m_pBackbufferQueue  = pDevice->getBackbufferQueue();
    m_pDevice           = pDevice;

    buildFrameResources(Vulkan::getResourceFormat(vkFormat));
    queryCommandPools();

    VkSemaphore imageAvailableSema = getWaitSemaphore(m_currentFrameIndex);

    result = vkAcquireNextImageKHR
                (
                    m_pDevice->get(), 
                    m_swapchain, 
                    UINT64_MAX, 
                    imageAvailableSema, 
                    VK_NULL_HANDLE, 
                    &m_currentImageIndex
                );

    if (result != VK_SUCCESS) 
    {
        R_WARN(R_CHANNEL_VULKAN, "AcquireNextImage was not successful...");    

        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}


ErrType VulkanSwapchain::onRebuild()
{
    // Destroy and rebuild the swapchain.
    destroy();
    build(m_pDevice);

    return RecluseResult_NoImpl;
}


ErrType VulkanSwapchain::destroy()
{
    VkDevice device     = m_pDevice->get();
    VkInstance instance = m_pDevice->getAdapter()->getInstance()->get();    

    if (m_swapchain) 
    {
        m_rawFrames.destroy(this);

        for (U32 i = 0; i < m_frameResources.size(); ++i) 
        {   
            m_frameViews[i]->release(m_pDevice);
            // Do not call m_frameResources[i]->destroy(), images are originally handled
            // by the swapchain.
            delete m_frameViews[i];
            delete m_frameResources[i];
        }

        vkDestroySwapchainKHR(device, m_swapchain, nullptr);    
        m_swapchain = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed swapchain.");
    }

    for (U32 i = 0; i < m_commandbuffers.size(); ++i) 
    {
        vkFreeCommandBuffers
            (
                device, 
                m_queueFamily->commandPools[i],
                1, 
                &m_commandbuffers[i]
            );   
    }

    return RecluseResult_Ok;
}


ErrType VulkanSwapchain::present(PresentConfig config)
{
    R_ASSERT(m_pBackbufferQueue != NULL);
    VkResult result = VK_SUCCESS;
    ErrType err = RecluseResult_Ok;

    // Flush all copies down for this run.
    m_pDevice->flushAllMappedRanges();
    m_pDevice->invalidateAllMappedRanges();

    if (config & PresentConfig_DelayPresent)
    {
        VulkanContext* pContext = static_cast<VulkanContext*>(m_pDevice->getContext());
        VkSubmitInfo info       = { };
        info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vkQueueSubmit(m_pBackbufferQueue->get(), 1, &info, pContext->getCurrentFence());
        return RecluseResult_Ok;
    }

    submitCommandsForPresenting();

    VkSwapchainKHR swapchains[]     = { m_swapchain };
    VkSemaphore pWaitSemaphores[]   = { getSignalSemaphore(m_currentFrameIndex) };
    VkDevice device                 = m_pDevice->get();
    VkSemaphore imageAvailableSema  = VK_NULL_HANDLE;
    
    if (!(config & PresentConfig_SkipPresent))
    {
        VkPresentInfoKHR info       = { };
        info.swapchainCount         = 1;
        info.pSwapchains            = swapchains;
        info.sType                  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.pWaitSemaphores        = pWaitSemaphores;
        info.waitSemaphoreCount     = 1;
        info.pResults               = nullptr;
        info.pImageIndices          = &m_currentImageIndex;
    
        result = vkQueuePresentKHR(m_pBackbufferQueue->get(), &info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
        {
            err = RecluseResult_NeedsUpdate;
        }
    }

    incrementFrameIndex();
    imageAvailableSema = getWaitSemaphore(m_currentFrameIndex);

    result = vkAcquireNextImageKHR
                (
                    m_pDevice->get(), 
                    m_swapchain, 
                    UINT64_MAX, 
                    imageAvailableSema, 
                    VK_NULL_HANDLE, 
                    &m_currentImageIndex
                );

    if (result != VK_SUCCESS) 
    {
        R_WARN(R_CHANNEL_VULKAN, "AcquireNextImage was not successful...");    

        err = RecluseResult_Failed;
    }

    return err;
}


void VulkanSwapchain::buildFrameResources(ResourceFormat resourceFormat)
{
    m_rawFrames.build(this);

    U32 numMaxFrames                                = m_rawFrames.getNumMaxFrames();
    const SwapchainCreateDescription& swapchainDesc = getDesc();

    m_frameResources.resize(numMaxFrames);
    m_frameViews.resize(numMaxFrames);

    // For swapchain resources, we don't necessarily need to allocate or create the native handles
    // for our images, we just need to pass them over to the object wrapper. This will then be 
    // used to pass as reference back to the high level API.
    //
    // We do need to create our view resources however.
    for (U32 i = 0; i < numMaxFrames; ++i) 
    {
        VkImage frame = m_rawFrames.getImage(i);

        GraphicsResourceDescription desc = { };
        desc.width          = swapchainDesc.renderWidth;
        desc.height         = swapchainDesc.renderHeight;
        desc.dimension      = ResourceDimension_2d;
        desc.depth          = 1;
        desc.format         = resourceFormat;
        desc.memoryUsage    = ResourceMemoryUsage_GpuOnly;
        desc.mipLevels      = 1;
        desc.usage          = ResourceUsage_RenderTarget;
        desc.samples        = 1;
        desc.arrayLevels    = 1;

        m_frameResources[i] = new VulkanImage(desc, frame, VK_IMAGE_LAYOUT_UNDEFINED);

        ResourceViewDescription viewDesc = { };
        viewDesc.format         = resourceFormat;
        viewDesc.pResource      = m_frameResources[i];
        viewDesc.mipLevelCount  = 1;
        viewDesc.layerCount     = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.baseMipLevel   = 0;
        viewDesc.dimension      = ResourceViewDimension_2d;
        viewDesc.type           = ResourceViewType_RenderTarget;
        
        m_frameViews[i] = new VulkanResourceView(viewDesc);
        m_frameViews[i]->initialize(m_pDevice);
    }
}


void VulkanSwapchain::submitCommandsForPresenting()
{
    VulkanContext* pContext             = m_pDevice->getContext()->castTo<VulkanContext>();
    VulkanImage* frame                  = m_frameResources[m_currentFrameIndex];
    VulkanPrimaryCommandList* pCmdList  = pContext->getPrimaryCommandList();
    VkDevice device                     = m_pDevice->get();
    U32 bufferIdx                       = pContext->getCurrentBufferIndex();
    VkImageMemoryBarrier imgBarrier     = { };
    VkImageSubresourceRange range       = { };
    VkCommandBuffer primaryCmdBuf       = pCmdList->get();
    VkCommandBuffer singleUseCmdBuf     = m_commandbuffers[bufferIdx];
    VkFence fence                       = pContext->getCurrentFence();
    VkSemaphore signalSemaphore         = m_rawFrames.getSignalSemaphore(m_currentFrameIndex);
    VkSemaphore waitSemaphore           = m_rawFrames.getWaitSemaphore(m_currentFrameIndex);

    R_ASSERT(primaryCmdBuf != NULL);

    if (pCmdList->getStatus() != CommandList_Ready) 
    {
        pCmdList->begin();
        pCmdList->end();
    }

    if (frame->getCurrentLayout() == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) 
    {
        // Push an empty submittal, in order to signal the semaphores.
        VkSubmitInfo submitInfo             = { };
        VkPipelineStageFlags waitStages[]   = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
        submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.signalSemaphoreCount     = 1;
        submitInfo.commandBufferCount       = 1;
        submitInfo.pSignalSemaphores        = &signalSemaphore;
        submitInfo.waitSemaphoreCount       = 1;
        submitInfo.pWaitSemaphores          = &waitSemaphore;
        submitInfo.pCommandBuffers          = &primaryCmdBuf;
        submitInfo.pWaitDstStageMask        = waitStages;

        vkQueueSubmit(m_pBackbufferQueue->get(), 1, &submitInfo, fence);

        return;
    }
    
    range.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseArrayLayer    = 0;
    range.baseMipLevel      = 0;
    range.layerCount        = 1;
    range.levelCount        = 1;

    imgBarrier = frame->transition(ResourceState_Present, range);    

    {
        VkCommandBufferBeginInfo begin = { };
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(singleUseCmdBuf, &begin);
    }

    vkCmdPipelineBarrier
        (
            singleUseCmdBuf, 
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
            VK_DEPENDENCY_BY_REGION_BIT, 
            0, nullptr, 
            0, nullptr, 
            1, &imgBarrier
        );

    vkEndCommandBuffer(singleUseCmdBuf);

    VkSubmitInfo submitInfo             = { };
    VkPipelineStageFlags waitStages[]   = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
    VkCommandBuffer buffers[2]          = { primaryCmdBuf, singleUseCmdBuf };
    submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount     = 1; 
    submitInfo.commandBufferCount       = 2;
    submitInfo.pSignalSemaphores        = &signalSemaphore;
    submitInfo.waitSemaphoreCount       = 1;
    submitInfo.pWaitSemaphores          = &waitSemaphore;
    submitInfo.pCommandBuffers          = buffers;
    submitInfo.pWaitDstStageMask        = waitStages;

    vkQueueSubmit(m_pBackbufferQueue->get(), 1, &submitInfo, fence);
}


void VulkanSwapchain::queryCommandPools()
{
    const std::vector<QueueFamily>& queueFamilies = m_pDevice->getQueueFamilies();
    VkDevice device = m_pDevice->get();

    for (U32 i = 0; i < queueFamilies.size(); ++i) 
    {
        if (queueFamilies[i].flags & (QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_COPY)) 
        {
            m_commandbuffers.resize(queueFamilies[i].commandPools.size());
            m_queueFamily = &queueFamilies[i];

            for (U32 j = 0; j < queueFamilies[i].commandPools.size(); ++j) 
            {    
                VkCommandBufferAllocateInfo allocIf = { };
                allocIf.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocIf.commandBufferCount          = 1;
                allocIf.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocIf.commandPool                 = queueFamilies[i].commandPools[j];
                vkAllocateCommandBuffers(device, &allocIf, &m_commandbuffers[j]);
            }

            break;
        }
    }
}
} // Recluse