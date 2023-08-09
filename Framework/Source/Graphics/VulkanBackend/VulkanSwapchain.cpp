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
    return m_frameImages[idx];
}


ResultCode VulkanSwapchain::build(VulkanDevice* pDevice)
{
    VkSwapchainCreateInfoKHR createInfo         = { };
    const SwapchainCreateDescription& pDesc     = getDesc();
    VkSurfaceKHR surface                        = pDevice->getSurface();
    VkResult result                             = VK_SUCCESS;
    VkFormat surfaceFormat                      = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace                  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    m_pBackbufferQueue                          = pDevice->getBackbufferQueue();
    m_pDevice                                   = pDevice;

    if (pDesc.renderWidth <= 0 || pDesc.renderHeight <= 0) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Can not define a RenderWidth or Height less than 1!");

        return RecluseResult_InvalidArgs;    
    }

    {
        VulkanAdapter* pAdapter                             = pDevice->getAdapter();
        std::vector<VkSurfaceFormatKHR> supportedFormats    = pAdapter->getSurfaceFormats(surface);
        VkFormat wantedSurfaceFormat                        = Vulkan::getVulkanFormat(pDesc.format);
        for (VkSurfaceFormatKHR& sF : supportedFormats) 
        {
            if (sF.format == wantedSurfaceFormat) 
            {
                colorSpace = sF.colorSpace;
                surfaceFormat = sF.format;
                break;
            }
        }

        if (surfaceFormat == VK_FORMAT_UNDEFINED) 
        {
            // Use the default first.
            R_WARN
                (
                    R_CHANNEL_VULKAN, 
                    "Can not find the requested format (%s), Using default supported surface format (%s).", 
                    getResourceFormatString(pDesc.format), 
                    getResourceFormatString(Vulkan::getResourceFormat(supportedFormats[0].format))
                );
            R_ASSERT_FORMAT(!supportedFormats.empty(), "No supported formats were found for this physical device!");
            surfaceFormat    = supportedFormats[0].format;
            colorSpace       = supportedFormats[0].colorSpace;
        }
    }
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = colorSpace;
    createInfo.imageFormat      = surfaceFormat;
    createInfo.surface          = surface;
    createInfo.imageExtent      = { pDesc.renderWidth, pDesc.renderHeight };
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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

    createInfo.presentMode = checkAvailablePresentMode(surface, createInfo.presentMode);
    
    result = vkCreateSwapchainKHR(pDevice->get(), &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) 
    {    
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create swapchain!");

        return RecluseResult_Failed;
    }    

    R_DEBUG(R_CHANNEL_VULKAN, "Successfully created vulkan swapchain!");

    requestOverrideResourceFormat(Vulkan::getResourceFormat(surfaceFormat));
    buildFrameResources(Vulkan::getResourceFormat(surfaceFormat));
    queryCommandPools();

    //VkSemaphore imageAvailableSema = getWaitSemaphore(m_currentFrameIndex);

    //result = vkAcquireNextImageKHR
    //            (
    //                m_pDevice->get(), 
    //                m_swapchain, 
    //                UINT64_MAX, 
    //                imageAvailableSema, 
    //                VK_NULL_HANDLE, 
    //                &m_currentImageIndex
    //            );

    //if (result != VK_SUCCESS) 
    //{
    //    R_WARN(R_CHANNEL_VULKAN, "AcquireNextImage was not successful...");    

    //    return RecluseResult_Failed;
    //}

    return RecluseResult_Ok;
}


ResultCode VulkanSwapchain::onRebuild()
{
    // Destroy and rebuild the swapchain.
    destroy();
    build(m_pDevice);
    // TODO(Garcia): Application frame index has to match with native API image index
    //                  This might need to be reinvestigated, as I feel either we need to remove one of these,
    //                  or syncronize them both.
    m_currentFrameIndex = 0;
    m_currentImageIndex = 0;
    return RecluseResult_Ok;
}


ResultCode VulkanSwapchain::destroy()
{
    VkDevice device     = m_pDevice->get();
    VkInstance instance = m_pDevice->getAdapter()->getInstance()->get();    

    if (m_swapchain) 
    {
        m_frameResources.destroy(this);

        for (U32 i = 0; i < m_frameImages.size(); ++i) 
        {   
            // Do not call m_frameResources[i]->destroy(), images are originally handled
            // by the swapchain.
            m_frameImages[i]->releaseViews();
            delete m_frameImages[i];
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
                m_commandPool,
                1, 
                &m_commandbuffers[i]
            );   
    }

    vkDestroyCommandPool(device, m_commandPool, nullptr);

    return RecluseResult_Ok;
}


ResultCode VulkanSwapchain::present(PresentConfig config)
{
    R_ASSERT(m_pBackbufferQueue != NULL);
    VkResult result                 = VK_SUCCESS;
    ResultCode err                     = RecluseResult_Ok;

    const U32 currentFrameIndex     = getCurrentFrameIndex();
    VkFence frameFence              = m_frameResources.getFence(currentFrameIndex);

    if (config & PresentConfig_DelayPresent)
    {
        return RecluseResult_Ok;
    }

    VkSwapchainKHR swapchains[]     = { m_swapchain };
    VkSemaphore pWaitSemaphores[]   = { getSignalSemaphore(m_currentFrameIndex) };
    VkDevice device                 = m_pDevice->get();
    
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

    return err;
}


ResultCode VulkanSwapchain::prepareFrame(VkFence cpuFence)
{
    R_ASSERT(m_pBackbufferQueue != NULL);
    VkResult result                 = VK_SUCCESS;
    const U32 currentFrameIndex     = getCurrentFrameIndex();
    VkFence frameFence              = cpuFence ? cpuFence : m_frameResources.getFence(currentFrameIndex);
    VkSemaphore imageAvailableSema  = getWaitSemaphore(currentFrameIndex);
    ResultCode err = RecluseResult_Ok;

    vkWaitForFences(m_pDevice->get(), 1, &frameFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_pDevice->get(), 1, &frameFence);

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
    m_frameResources.build(this);

    U32 numMaxFrames                                = m_frameResources.getNumMaxFrames();
    const SwapchainCreateDescription& swapchainDesc = getDesc();

    m_frameImages.resize(numMaxFrames);

    // For swapchain resources, we don't necessarily need to allocate or create the native handles
    // for our images, we just need to pass them over to the object wrapper. This will then be 
    // used to pass as reference back to the high level API.
    //
    // We do need to create our view resources however.
    for (U32 i = 0; i < numMaxFrames; ++i) 
    {
        VkImage frame = m_frameResources.getImage(i);

        GraphicsResourceDescription desc = { };
        desc.width          = swapchainDesc.renderWidth;
        desc.height         = swapchainDesc.renderHeight;
        desc.dimension      = ResourceDimension_2d;
        desc.format         = resourceFormat;
        desc.memoryUsage    = ResourceMemoryUsage_GpuOnly;
        desc.mipLevels      = 1;
        desc.usage          = ResourceUsage_RenderTarget;
        desc.samples        = 1;
        desc.depthOrArraySize    = 1;

        m_frameImages[i] = new VulkanImage(desc, frame, VK_IMAGE_LAYOUT_UNDEFINED);
        m_frameImages[i]->generateId();
        m_frameImages[i]->setDevice(m_pDevice);
    }
}


void VulkanSwapchain::queryCommandPools()
{
    const std::vector<QueueFamily>& queueFamilies = m_pDevice->getQueueFamilies();
    VkDevice device = m_pDevice->get();

    for (U32 i = 0; i < queueFamilies.size(); ++i) 
    {
        if (queueFamilies[i].flags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) 
        {
            m_commandbuffers.resize(m_frameResources.getNumMaxFrames());
            m_queueFamily = &queueFamilies[i];

            {
                VkCommandPoolCreateInfo poolCreateInfo = { };
                poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolCreateInfo.queueFamilyIndex = m_queueFamily->queueFamilyIndex;

                vkCreateCommandPool(device, &poolCreateInfo, nullptr, &m_commandPool);
            }

            for (U32 j = 0; j < m_commandbuffers.size(); ++j) 
            {    
                VkCommandBufferAllocateInfo allocIf = { };
                allocIf.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocIf.commandBufferCount          = 1;
                allocIf.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocIf.commandPool                 = m_commandPool;
                vkAllocateCommandBuffers(device, &allocIf, &m_commandbuffers[j]);
            }

            break;
        }
    }

    // For the back buffer queue to be able to handle one time only command buffers, set it up from
    // the swapchain command pool.
    m_pBackbufferQueue->setTemporaryCommandPoolUse(m_commandPool);
}


ResultCode VulkanSwapchain::submitFinalCommandBuffer(VkCommandBuffer commandBuffer, VkFence cpuFence)
{
    VulkanImage* frame                  = m_frameImages[m_currentFrameIndex];
    VkDevice device                     = m_pDevice->get();
    VkImageMemoryBarrier imgBarrier     = { };
    VkImageSubresourceRange range       = { };
    VkCommandBuffer primaryCmdBuf       = commandBuffer;
    VkCommandBuffer singleUseCmdBuf     = m_commandbuffers[m_currentFrameIndex];
    VkFence fence                       = cpuFence ? cpuFence : m_frameResources.getFence(m_currentFrameIndex);
    VkSemaphore signalSemaphore         = m_frameResources.getSignalSemaphore(m_currentFrameIndex);
    VkSemaphore waitSemaphore           = m_frameResources.getWaitSemaphore(m_currentFrameIndex);

    R_ASSERT(primaryCmdBuf != NULL);

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

        return RecluseResult_Ok;
    }

    R_WARN
        (
            R_CHANNEL_VULKAN, 
            "Submitting command buffer %#010llx without transitioning back swapchain buffer to PRESENT!"
            " Will require an additional command buffer to transition it.",
            reinterpret_cast<UPtr>(commandBuffer)
        );
    
    range.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseArrayLayer    = 0;
    range.baseMipLevel      = 0;
    range.layerCount        = 1;
    range.levelCount        = 1;

    imgBarrier = frame->transition(ResourceState_Present, range);    

    vkResetCommandBuffer(singleUseCmdBuf, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

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
    return RecluseResult_Ok;
}


VkPresentModeKHR VulkanSwapchain::checkAvailablePresentMode(VkSurfaceKHR surface, VkPresentModeKHR presentMode)
{
    Bool foundSupportedPresentMode                      = false;
    VkPresentModeKHR supportedPresentMode               = presentMode;
    std::vector<VkPresentModeKHR> supportedPresentModes = m_pDevice->getAdapter()->getSupportedPresentModes(surface);

    R_ASSERT_FORMAT(!supportedPresentModes.empty(), "If trying to create a swapchain with a surface, there should at least be one supported present mode!");
    
    for (U32 i = 0; i < supportedPresentModes.size(); ++i)
    {
        if (supportedPresentMode == supportedPresentModes[i])
        {
            foundSupportedPresentMode = true;
            break;
        }
    }

    if (!foundSupportedPresentMode)
    {
        R_WARN(R_CHANNEL_VULKAN, "Could not find the requested supported present mode... resorting to default present mode...");
        supportedPresentMode = supportedPresentModes[0];
    }

    return supportedPresentMode;
}
} // Recluse