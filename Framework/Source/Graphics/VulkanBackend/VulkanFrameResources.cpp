//
#include "VulkanFrameResources.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


void VulkanFrameResources::build(VulkanSwapchain* pSwapchain)
{
    VkDevice device             = pSwapchain->getDevice()->get();
    VkSwapchainKHR swapchain    = pSwapchain->get();
    U32 swapchainImageCount     = 0;
    VkResult result             = VK_SUCCESS;

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);

    m_frames.resize(swapchainImageCount);
    m_frameBuffers.resize(swapchainImageCount);
    m_frameSignalSemaphores.resize(swapchainImageCount);
    m_frameWaitSemaphores.resize(swapchainImageCount);
    m_frameFences.resize(swapchainImageCount);

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, m_frames.data());

    VkSemaphoreCreateInfo semaIf = { };
    semaIf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaIf.flags = 0;

    VkFenceCreateInfo fenceIf = { };
    fenceIf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceIf.flags = 0;

    for (U32 i = 0; i < m_frameWaitSemaphores.size(); ++i) {
    
        vkCreateSemaphore(device, &semaIf, nullptr, &m_frameWaitSemaphores[i]);
        vkCreateSemaphore(device, &semaIf, nullptr, &m_frameSignalSemaphores[i]);
        vkCreateFence(device, &fenceIf, nullptr, &m_frameFences[i]);
    
    }
    
}


void VulkanFrameResources::destroy(VulkanSwapchain* pSwapchain)
{
    VkDevice device = pSwapchain->getDevice()->get();

    for (U32 i = 0; i < m_frameSignalSemaphores.size(); ++i) {
    
        vkDestroySemaphore(device, m_frameSignalSemaphores[i], nullptr);
        vkDestroySemaphore(device, m_frameWaitSemaphores[i], nullptr);
        vkDestroyFence(device, m_frameFences[i], nullptr);
    
    }
}
} // Recluse