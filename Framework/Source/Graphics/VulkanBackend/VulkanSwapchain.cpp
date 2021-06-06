// 
#include "Core/Types.hpp"
#include "VulkanSwapchain.hpp"
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


U32 VulkanSwapchain::build(VkDevice device, U32 desiredFrames, U32 renderWidth, U32 renderHeight)
{
    VkSwapchainCreateInfoKHR createInfo = { };
    VkResult result                     = VK_SUCCESS;
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat      = VK_FORMAT_B8G8R8A8_SRGB;
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) {
        
        R_ERR(R_CHANNEL_VULKAN, "Failed to create swapchain!");

        return -1;
    }    

    return 0;
}
} // Recluse