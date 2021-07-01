// 
#include "Core/Types.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
#include "VulkanQueue.hpp"
#include "VulkanAdapter.hpp"
#include "Core/Messaging.hpp"


namespace Recluse {


GraphicsResource* VulkanSwapchain::getFrame(U32 idx)
{
    return nullptr;
}


GraphicsResourceView* VulkanSwapchain::getFrameView(U32 idx)
{
    return nullptr;
}


ErrType VulkanSwapchain::build(VulkanDevice* pDevice, const SwapchainCreateDescription& pDesc)
{
    VkSwapchainCreateInfoKHR createInfo = { };
    VkResult result                     = VK_SUCCESS;
    
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
    
    result = vkCreateSwapchainKHR(pDevice->get(), &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) {
        
        R_ERR(R_CHANNEL_VULKAN, "Failed to create swapchain!");

        return REC_RESULT_FAILED;
    }    

    R_DEBUG(R_CHANNEL_VULKAN, "Successfully created vulkan swapchain!");
    
    m_pBackbufferQueue  = static_cast<VulkanQueue*>(pDesc.pBackbufferQueue);
    m_pDevice           = pDevice;

    buildFrameResources();

    return REC_RESULT_OK;
}


ErrType VulkanSwapchain::rebuild(const SwapchainCreateDescription& pDesc)
{

    const VulkanContext* pVc    = m_pDevice->getAdapter()->getContext();

    destroy(pVc->get(), m_pDevice->get());

    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType VulkanSwapchain::destroy(VkInstance instance, VkDevice device)
{
    if (m_swapchain) {

        m_frameResources.destroy(this);

        vkDestroySwapchainKHR(device, m_swapchain, nullptr);    
        m_swapchain = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed swapchain.");

    }

    return REC_RESULT_OK;
}


ErrType VulkanSwapchain::present()
{
    R_ASSERT(m_pBackbufferQueue != NULL);
    
    ErrType err                 = REC_RESULT_OK;
    VkSwapchainKHR swapchains[] = { m_swapchain };
    VkPresentInfoKHR info       = { };
    
    info.swapchainCount         = 1;
    info.pSwapchains            = swapchains;
    info.sType                  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pWaitSemaphores        = nullptr;
    info.waitSemaphoreCount     = 0;
    info.pResults               = nullptr;
    info.pImageIndices          = &m_currentFrameIndex;
    
    VkResult result = vkQueuePresentKHR(m_pBackbufferQueue->get(), &info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    
        err = REC_RESULT_NEEDS_UPDATE;
    
    }

    result = vkAcquireNextImageKHR(m_pDevice->get(), m_swapchain, UINT64_MAX, 
        VK_NULL_HANDLE, VK_NULL_HANDLE, &m_currentFrameIndex);

    if (result != VK_SUCCESS) {

        R_WARN(R_CHANNEL_VULKAN, "AcquireNextImage was not successful...");    

        err = REC_RESULT_FAILED;
    
    }

    R_ASSERT(m_pDevice->getBufferCount() > 0);

    m_pDevice->incrementBufferIndex();

    m_pDevice->prepare();
    
    return err;
}


void VulkanSwapchain::buildFrameResources()
{
    m_frameResources.build(this);
}
} // Recluse