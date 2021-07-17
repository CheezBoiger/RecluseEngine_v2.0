// 
#include "Recluse/Types.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
#include "VulkanQueue.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
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
    VkResult result                             = VK_SUCCESS;
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat      = VK_FORMAT_B8G8R8A8_SRGB;
    createInfo.surface          = pDevice->getSurface();
    createInfo.imageExtent      = { pDesc.renderWidth, pDesc.renderHeight };
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.minImageCount    = pDesc.desiredFrames;
    createInfo.imageArrayLayers = 1;
    createInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; 
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.queueFamilyIndexCount = 0;

    switch (pDesc.buffering) {
        default:
        case FRAME_BUFFERING_SINGLE: createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; break;
        case FRAME_BUFFERING_DOUBLE: createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; break;
        case FRAME_BUFFERING_TRIPLE: createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; break;
    }
    
    result = vkCreateSwapchainKHR(pDevice->get(), &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) {
        
        R_ERR(R_CHANNEL_VULKAN, "Failed to create swapchain!");

        return REC_RESULT_FAILED;
    }    

    R_DEBUG(R_CHANNEL_VULKAN, "Successfully created vulkan swapchain!");
    
    m_pBackbufferQueue  = static_cast<VulkanQueue*>(pDesc.pBackbufferQueue);
    m_pDevice           = pDevice;

    buildFrameResources();
    queryCommandPools();

    VkSemaphore imageAvailableSema = getWaitSemaphore(m_currentFrameIndex);
    result = vkAcquireNextImageKHR(m_pDevice->get(), m_swapchain, UINT64_MAX, 
        imageAvailableSema, VK_NULL_HANDLE, &m_currentImageIndex);

    if (result != VK_SUCCESS) {

        R_WARN(R_CHANNEL_VULKAN, "AcquireNextImage was not successful...");    

        return REC_RESULT_FAILED;
    
    }

    return REC_RESULT_OK;
}


ErrType VulkanSwapchain::onRebuild()
{
    // Destroy and rebuild the swapchain.
    destroy();
    build(m_pDevice);

    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType VulkanSwapchain::destroy()
{
    VkDevice device     = m_pDevice->get();
    VkInstance instance = m_pDevice->getAdapter()->getContext()->get();    

    if (m_swapchain) {

        m_rawFrames.destroy(this);

        for (U32 i = 0; i < m_frameResources.size(); ++i) {
        
            m_frameViews[i]->destroy(m_pDevice);
            // Do not call m_frameResources[i]->destroy(), images are originally handled
            // by the swapchain.
            delete m_frameViews[i];
            delete m_frameResources[i];
        }

        vkDestroySwapchainKHR(device, m_swapchain, nullptr);    
        m_swapchain = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed swapchain.");

    }

    for (U32 i = 0; i < m_commandbuffers.size(); ++i) {
    
        vkFreeCommandBuffers(device, m_queueFamily->commandPools[i],
            1, &m_commandbuffers[i]);    
        
    }

    return REC_RESULT_OK;
}


ErrType VulkanSwapchain::present()
{
    R_ASSERT(m_pBackbufferQueue != NULL);

    transitionCurrentFrameToPresentable();
 
    ErrType err                     = REC_RESULT_OK;
    VkSwapchainKHR swapchains[]     = { m_swapchain };
    VkPresentInfoKHR info           = { };
    VkSemaphore pWaitSemaphores[]   = { getSignalSemaphore(m_currentFrameIndex) };
    VkDevice device                 = m_pDevice->get();
    VkFence frameFence              = VK_NULL_HANDLE;
    VkSemaphore imageAvailableSema  = VK_NULL_HANDLE;
    
    info.swapchainCount         = 1;
    info.pSwapchains            = swapchains;
    info.sType                  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pWaitSemaphores        = pWaitSemaphores;
    info.waitSemaphoreCount     = 1;
    info.pResults               = nullptr;
    info.pImageIndices          = &m_currentImageIndex;
    
    VkResult result = vkQueuePresentKHR(m_pBackbufferQueue->get(), &info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    
        err = REC_RESULT_NEEDS_UPDATE;
    
    }

    m_pDevice->incrementBufferIndex();

    frameFence = m_pDevice->getCurrentFence();

    incrementFrameIndex();
    imageAvailableSema = getWaitSemaphore(m_currentFrameIndex);

    result = vkAcquireNextImageKHR(m_pDevice->get(), m_swapchain, UINT64_MAX, 
        imageAvailableSema, VK_NULL_HANDLE, &m_currentImageIndex);

    if (result != VK_SUCCESS) {

        R_WARN(R_CHANNEL_VULKAN, "AcquireNextImage was not successful...");    

        err = REC_RESULT_FAILED;
    
    }

    result = vkWaitForFences(device, 1, &frameFence, VK_TRUE, UINT16_MAX);

    if (result != REC_RESULT_OK) {
    
        R_WARN(R_CHANNEL_VULKAN, "Fence wait failed...");
    
    }

    vkResetFences(device, 1, &frameFence);

    R_ASSERT(m_pDevice->getBufferCount() > 0);

    m_pDevice->prepare();

    return err;
}


void VulkanSwapchain::buildFrameResources()
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
    for (U32 i = 0; i < numMaxFrames; ++i) {
    
        VkImage frame = m_rawFrames.getImage(i);

        GraphicsResourceDescription desc = { };
        desc.width          = swapchainDesc.renderWidth;
        desc.height         = swapchainDesc.renderHeight;
        desc.dimension      = RESOURCE_DIMENSION_2D;
        desc.depth          = 1;
        desc.format         = RESOURCE_FORMAT_B8G8R8A8_SRGB;
        desc.memoryUsage    = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        desc.mipLevels      = 1;
        desc.usage          = RESOURCE_USAGE_RENDER_TARGET;
        desc.samples        = 1;
        desc.arrayLevels    = 1;

        m_frameResources[i] = new VulkanImage(desc, frame, VK_IMAGE_LAYOUT_UNDEFINED);

        ResourceViewDesc viewDesc = { };
        viewDesc.format         = RESOURCE_FORMAT_B8G8R8A8_SRGB;
        viewDesc.pResource      = m_frameResources[i];
        viewDesc.mipLevelCount  = 1;
        viewDesc.layerCount     = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.baseMipLevel   = 0;
        viewDesc.dimension      = RESOURCE_VIEW_DIMENSION_2D;
        viewDesc.type           = RESOURCE_VIEW_TYPE_RENDER_TARGET;
        
        m_frameViews[i] = new VulkanResourceView(viewDesc);
        m_frameViews[i]->initialize(m_pDevice);
    
    }
}


void VulkanSwapchain::transitionCurrentFrameToPresentable()
{
    // TODO: Likely want this to be recorded once...

    VulkanImage* frame              = m_frameResources[m_currentFrameIndex];
    VkDevice device                 = m_pDevice->get();
    U32 bufferIdx                   = m_pDevice->getCurrentBufferIndex();
    VkImageMemoryBarrier imgBarrier = { };
    VkImageSubresourceRange range   = { };
    VkCommandBuffer singleUseCmdBuf = m_commandbuffers[bufferIdx];
    VkFence fence                   = m_pDevice->getCurrentFence();
    VkSemaphore signalSemaphore     = m_rawFrames.getSignalSemaphore(m_currentFrameIndex);
    VkSemaphore waitSemaphore       = m_rawFrames.getWaitSemaphore(m_currentFrameIndex);

    if (frame->getCurrentLayout() == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    
        return;
    
    }
    
    range.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseArrayLayer    = 0;
    range.baseMipLevel      = 0;
    range.layerCount        = 1;
    range.levelCount        = 1;

    imgBarrier = frame->transition(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range);    

    {
        VkCommandBufferBeginInfo begin = { };
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(singleUseCmdBuf, &begin);
    }

    vkCmdPipelineBarrier(singleUseCmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, 
        &imgBarrier);

    vkEndCommandBuffer(singleUseCmdBuf);

    VkSubmitInfo submitInfo             = { };
    VkPipelineStageFlags waitStages[]   = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
    submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount     = 1; 
    submitInfo.commandBufferCount       = 1;
    submitInfo.pSignalSemaphores        = &signalSemaphore;
    submitInfo.waitSemaphoreCount       = 1;
    submitInfo.pWaitSemaphores          = &waitSemaphore;
    submitInfo.pCommandBuffers          = &singleUseCmdBuf;
    submitInfo.pWaitDstStageMask        = waitStages;

    vkQueueSubmit(m_pBackbufferQueue->get(), 1, &submitInfo, fence);
}


void VulkanSwapchain::queryCommandPools()
{
    const std::vector<QueueFamily>& queueFamilies = m_pDevice->getQueueFamilies();
    VkDevice device = m_pDevice->get();

    for (U32 i = 0; i < queueFamilies.size(); ++i) {
    
        if (queueFamilies[i].flags & (QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_COPY)) {

            m_commandbuffers.resize(queueFamilies[i].commandPools.size());
            m_queueFamily = &queueFamilies[i];

            for (U32 j = 0; j < queueFamilies[i].commandPools.size(); ++j) {
            
                VkCommandBufferAllocateInfo allocIf = { };
                allocIf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocIf.commandBufferCount = 1;
                allocIf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocIf.commandPool  = queueFamilies[i].commandPools[j];
                vkAllocateCommandBuffers(device, &allocIf, &m_commandbuffers[j]);
            
            }

            break;
        
        }

    }
}
} // Recluse