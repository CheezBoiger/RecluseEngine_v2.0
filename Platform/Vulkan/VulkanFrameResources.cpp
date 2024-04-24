//
#include "VulkanFrameResources.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


//void VulkanBufferResources::build(VulkanSwapchain* pSwapchain)
//{
//    VkDevice device             = pSwapchain->getDevice()->get();
//    VkSwapchainKHR swapchain    = pSwapchain->get();
//    U32 swapchainImageCount     = 0;
//    VkResult result             = VK_SUCCESS;
//
//    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
//
//    m_frames.resize(swapchainImageCount);
//    m_frameBuffers.resize(swapchainImageCount);
//    m_signalSemaphores.resize(swapchainImageCount);
//    m_waitSemaphores.resize(swapchainImageCount);
//    m_bufferFences.resize(swapchainImageCount);
//
//    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, m_frames.data());
//
//    VkSemaphoreCreateInfo semaIf    = { };
//    semaIf.sType                    = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//    semaIf.flags                    = 0;
//
//    VkFenceCreateInfo fenceIf       = { };
//    fenceIf.sType                   = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//    fenceIf.flags                   = VK_FENCE_CREATE_SIGNALED_BIT;
//    
//    for (U32 i = 0; i < m_waitSemaphores.size(); ++i) 
//    {
//        vkCreateSemaphore(device, &semaIf, nullptr, &m_waitSemaphores[i]);
//        vkCreateSemaphore(device, &semaIf, nullptr, &m_signalSemaphores[i]);
//        vkCreateFence(device, &fenceIf, nullptr, &m_bufferFences[i]);
//    }
//    
//}
//
//
//void VulkanBufferResources::destroy(VulkanSwapchain* pSwapchain)
//{
//    VkDevice device = pSwapchain->getDevice()->get();
//
//    for (U32 i = 0; i < m_signalSemaphores.size(); ++i) 
//    {
//        vkDestroySemaphore(device, m_signalSemaphores[i], nullptr);
//        vkDestroySemaphore(device, m_waitSemaphores[i], nullptr);
//        vkDestroyFence(device, m_bufferFences[i], nullptr);
//    }
//}
} // Recluse