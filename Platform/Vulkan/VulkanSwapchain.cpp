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
#include "Recluse/Math/MathCommons.hpp"


namespace Recluse {
namespace Vulkan {

GraphicsResource* VulkanSwapchain::getFrame(U32 idx)
{
    return m_frameImages[idx];
}


ResultCode VulkanSwapchain::build(VulkanDevice* pDevice, void* windowHandle)
{
    VkSwapchainCreateInfoKHR createInfo         = { };
    const SwapchainCreateDescription& pDesc     = getDesc();
    VkResult result                             = VK_SUCCESS;
    VkFormat surfaceFormat                      = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace                  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    U32 frameCount                              = pDesc.desiredFrames;
    U32 renderWidth                             = pDesc.renderWidth;
    U32 renderHeight                            = pDesc.renderHeight;
    VkImageUsageFlags imageUsageBits            = 0;
    VkSurfaceKHR surface                        = pDevice->getAdapter()->getInstance()->makeSurface(windowHandle);
    m_pDevice                                   = pDevice;
 
   if (pDesc.renderWidth <= 0 || pDesc.renderHeight <= 0) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Can not define a RenderWidth or Height less than 1! Width=%d, Height=%d", pDesc.renderWidth, pDesc.renderHeight);

        return RecluseResult_InvalidArgs;    
    }

    {
        VulkanAdapter* pAdapter                             = pDevice->getAdapter();
        std::vector<VkSurfaceFormatKHR> supportedFormats    = pAdapter->getSurfaceFormats(surface);
        VkFormat wantedSurfaceFormat                        = Vulkan::getVulkanFormat(pDesc.format);
        VkSurfaceCapabilitiesKHR capabilities               = pAdapter->getSurfaceCapabilities(surface);
        frameCount                                          = Math::clamp(frameCount, capabilities.minImageCount, capabilities.maxImageCount);
        renderWidth                                         = Math::clamp(renderWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        renderHeight                                        = Math::clamp(renderHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        if (renderWidth <= 0 || renderHeight <= 0)
        {
            R_ERROR
                (
                    R_CHANNEL_VULKAN, 
                    "Device capabilities seems to have returned zero values for min and max extents. Can not create swapchain! MinImageExtent=(%d, %d), MaxImageExtent=(%d, %d)", 
                    capabilities.minImageExtent.width, capabilities.minImageExtent.height, capabilities.maxImageExtent.width, capabilities.maxImageExtent.height
                );
            return RecluseResult_InvalidArgs;
        }
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

        if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) imageUsageBits |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT)          imageUsageBits |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)     imageUsageBits |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)     imageUsageBits |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = colorSpace;
    createInfo.imageFormat      = surfaceFormat;
    createInfo.surface          = surface;
    createInfo.imageExtent      = { renderWidth, renderHeight };
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage       = imageUsageBits;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.minImageCount    = frameCount;
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
    requestOverrideFrameCount(frameCount);
    requestOverrideRenderResolution(renderWidth, renderHeight);
    buildFrameResources(Vulkan::getResourceFormat(surfaceFormat));

    m_windowHandle = windowHandle;

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

    VkSurfaceKHR surface                        = m_pDevice->getAdapter()->getInstance()->makeSurface(m_windowHandle);
    ResultCode result                           = RecluseResult_Failed;
    if (surface)
    {
        VkSurfaceCapabilitiesKHR capabilities = m_pDevice->getAdapter()->getSurfaceCapabilities(surface);
        U32 renderWidth                                         = capabilities.currentExtent.width;
        U32 renderHeight                                        = capabilities.currentExtent.height;
        if (renderWidth > 0 && renderHeight > 0)
        {
            release();
            result = build(m_pDevice, m_windowHandle);
            // TODO(Garcia): Application frame index has to match with native API image index
            //                  This might need to be reinvestigated, as I feel either we need to remove one of these,
            //                  or syncronize them both.
            m_currentFrameIndex = 0;
            m_currentImageIndex = 0;
        }
        else
        {
            R_WARN(R_CHANNEL_VULKAN, "Vulkan surface capabilities returned 0 extent size. This could mean that the window is restoring from minimized state. OS should restore this on Proc.");
        }
    }
    return result;
}


ResultCode VulkanSwapchain::release()
{
    VkDevice device     = m_pDevice->get();
    if (m_swapchain) 
    {
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

    return RecluseResult_Ok;
}


ResultCode VulkanSwapchain::present(GraphicsContext* context)
{
    R_ASSERT(m_pBackbufferQueue != NULL);
    VulkanContext* vulkanContext        = context->castTo<VulkanContext>();
    const U32 contextIndex              = vulkanContext->getCurrentFrameIndex();
    VulkanContextFrame& contextFrame    = vulkanContext->getContextFrame(contextIndex);
    VkResult result                     = VK_SUCCESS;
    ResultCode err                      = RecluseResult_Ok;

    VkSwapchainKHR swapchains[]         = { m_swapchain };
    VkSemaphore pWaitSemaphores[]       = { contextFrame.signalSemaphore };
    VkDevice device                     = m_pDevice->get();

    R_ASSERT_FORMAT(contextIndex < m_frames.size(), "Context frame is larger than the actual number of swapchain images! This will cause problems!!");
    validateSwapchainImageIsPresentable();
    
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
        R_DEBUG(R_CHANNEL_VULKAN, "Swapchain is out of date, needs to be recreated...");    
        err = RecluseResult_NeedsUpdate;
    }

    incrementFrameIndex();
    return err;
}


void VulkanSwapchain::validateSwapchainImageIsPresentable()
{
    VulkanImage* frame                  = m_frameImages[m_currentImageIndex];

    if (frame->getCurrentLayout() != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) 
    {
        R_WARN
            (
                R_CHANNEL_VULKAN, 
                "Submitting command buffer on frame %d without transitioning back swapchain buffer to PRESENT!"
                " Will require a transition after all rendering is done!",
                m_currentImageIndex
            );
    }

    
    //range.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    //range.baseArrayLayer    = 0;
    //range.baseMipLevel      = 0;
    //range.layerCount        = 1;
    //range.levelCount        = 1;

    //imgBarrier = frame->transition(ResourceState_Present, range);    

    //VkCommandBuffer singleUseCmdBuf     = m_queue->;
    //vkResetCommandBuffer(singleUseCmdBuf, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    //{
    //    VkCommandBufferBeginInfo begin = { };
    //    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //    vkBeginCommandBuffer(singleUseCmdBuf, &begin);
    //}

    //vkCmdPipelineBarrier
    //    (
    //        singleUseCmdBuf, 
    //        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    //        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
    //        VK_DEPENDENCY_BY_REGION_BIT, 
    //        0, nullptr, 
    //        0, nullptr, 
    //        1, &imgBarrier
    //    );

    //vkEndCommandBuffer(singleUseCmdBuf);

    //VkSubmitInfo submitInfo             = { };
    //VkPipelineStageFlags waitStages[]   = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
    //VkCommandBuffer buffers[2]          = { primaryCmdBuf, singleUseCmdBuf };
    //submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //submitInfo.signalSemaphoreCount     = 1; 
    //submitInfo.commandBufferCount       = 2;
    //submitInfo.pSignalSemaphores        = &signalSemaphore;
    //submitInfo.waitSemaphoreCount       = 1;
    //submitInfo.pWaitSemaphores          = &waitSemaphore;
    //submitInfo.pCommandBuffers          = buffers;
    //submitInfo.pWaitDstStageMask        = waitStages;

    //vkQueueSubmit(m_pBackbufferQueue->get(), 1, &submitInfo, fence);
}


ResultCode VulkanSwapchain::prepare(GraphicsContext* context)
{
    //R_ASSERT_FORMAT(context != NULL, "Swapchain requires a context, in order to increment the next frame!");
    R_ASSERT(m_pBackbufferQueue != NULL);
    VulkanContext* vulkanContext    = context->castTo<VulkanContext>();
    R_ASSERT_FORMAT(vulkanContext->getFrameCount() <= m_frameImages.size(), "Context frame count is higher than the actual swapchain buffer count! This will cause a crash!");
    vulkanContext->begin();

    VulkanContextFrame& contextFrame    = vulkanContext->getContextFrame(vulkanContext->getCurrentFrameIndex());
    VkResult result                     = VK_SUCCESS;
    VkSemaphore imageAvailableSema      = contextFrame.waitSemaphore;
    ResultCode err                      = RecluseResult_Ok;

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
        switch (result)
        {
            case VK_ERROR_OUT_OF_DATE_KHR:
                R_DEBUG(R_CHANNEL_VULKAN, "Swapchain is out of date, needs to be recreated...");    
                err = RecluseResult_NeedsUpdate;
                break;
            default:
                R_WARN(R_CHANNEL_VULKAN, "Swapchain acquire next image was unsuccessfull. This may lead to errors!");
                break;
        }
        
    }

    return err;
}


void VulkanSwapchain::buildFrameResources(ResourceFormat resourceFormat)
{
    // Obtain the swapchain images.
    VkDevice device = m_pDevice->get();
    U32 swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(device, m_swapchain, &swapchainImageCount, nullptr);
    m_frames.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &swapchainImageCount, m_frames.data());

    U32 numMaxFrames                                = m_frames.size();
    const SwapchainCreateDescription& swapchainDesc = getDesc();

    m_frameImages.resize(numMaxFrames);

    // For swapchain resources, we don't necessarily need to allocate or create the native handles
    // for our images, we just need to pass them over to the object wrapper. This will then be 
    // used to pass as reference back to the high level API.
    //
    // We do need to create our view resources however.
    for (U32 i = 0; i < numMaxFrames; ++i) 
    {
        VkImage frame = m_frames[i];

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

        m_frameImages[i] = new VulkanImage(frame, VK_IMAGE_LAYOUT_UNDEFINED);
        m_frameImages[i]->initializeMetadata(desc);
        m_frameImages[i]->generateId();
        m_frameImages[i]->setDevice(m_pDevice);
    }
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
} // Vulkan
} // Recluse