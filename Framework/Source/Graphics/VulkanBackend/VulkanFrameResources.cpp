//
#include "VulkanFrameResources.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"

#include "Core/Messaging.hpp"

namespace Recluse {


void VulkanFrameResources::build(VulkanSwapchain* pSwapchain)
{
    VkDevice device             = pSwapchain->getDevice()->get();
    VkSwapchainKHR swapchain    = pSwapchain->get();
    U32 swapchainImageCount     = 0;
    VkResult result             = VK_SUCCESS;

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);

    m_frames.resize(swapchainImageCount);
    m_frameViews.resize(swapchainImageCount);
    m_frameBuffers.resize(swapchainImageCount);
    m_frameSignalSemaphores.resize(swapchainImageCount);
    m_frameWaitSemaphores.resize(swapchainImageCount);

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, m_frames.data());


    VkImageViewCreateInfo viewInfo = { };
    viewInfo.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.components.r                       = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g                       = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b                       = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a                       = VK_COMPONENT_SWIZZLE_A;
    viewInfo.format                             = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer    = 0;
    viewInfo.subresourceRange.baseMipLevel      = 0;
    viewInfo.subresourceRange.layerCount        = 1;
    viewInfo.subresourceRange.levelCount        = 1;

    for (U32 i = 0; i < swapchainImageCount; ++i) {

        viewInfo.image = m_frames[i];

        result = vkCreateImageView(device, &viewInfo, nullptr, &m_frameViews[i]);

        if (result != VK_SUCCESS) {
        
            R_ERR(R_CHANNEL_VULKAN, "Failed to create swapchain image view for index %d", i);
        
        }

    }
    
}


void VulkanFrameResources::destroy(VulkanSwapchain* pSwapchain)
{
    VkDevice device = pSwapchain->getDevice()->get();

    for (U32 i = 0; i < m_frameViews.size(); ++i) {
    
        vkDestroyImageView(device, m_frameViews[i], nullptr);
    
    }
}
} // Recluse