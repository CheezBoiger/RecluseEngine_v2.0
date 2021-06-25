// 
#include "Core/Types.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
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


ErrType VulkanSwapchain::build(VulkanDevice* pDevice, const SwapchainCreateDescription* pDesc)
{
    VkSwapchainCreateInfoKHR createInfo = { };
    VkResult result                     = VK_SUCCESS;
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat      = VK_FORMAT_B8G8R8A8_SRGB;
    createInfo.surface          = pDevice->getSurface();
    createInfo.imageExtent      = { pDesc->renderWidth, pDesc->renderHeight };
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.minImageCount    = pDesc->desiredFrames;
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

    return REC_RESULT_OK;
}


ErrType VulkanSwapchain::rebuild(const GraphicsContext* pContext, const GraphicsDevice* pDevice, const SwapchainCreateDescription* pDesc)
{
    R_ASSERT(((pContext != NULL) && (pDevice != NULL)));

    const VulkanContext* pVc    = static_cast<const VulkanContext*>(pContext);
    const VulkanDevice* pVd     = static_cast<const VulkanDevice*>(pDevice);

    destroy(pVc->get(), pVd->get());

    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType VulkanSwapchain::destroy(VkInstance instance, VkDevice device)
{
    if (m_swapchain) {

        vkDestroySwapchainKHR(device, m_swapchain, nullptr);    
        m_swapchain = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed swapchain.");

    }

    return REC_RESULT_OK;
}
} // Recluse