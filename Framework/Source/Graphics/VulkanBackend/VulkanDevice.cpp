//
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanSwapchain.hpp"
#include "Core/Messaging.hpp"

#include "Graphics/GraphicsAdapter.hpp"

namespace Recluse {


ErrType VulkanDevice::initialize(const VulkanAdapter& adapter, DeviceCreateInfo* info)
{
    const VulkanAdapter& nativeAdapter = static_cast<const VulkanAdapter&>(adapter);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 
    VkDeviceCreateInfo createInfo = { };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    std::vector<VkQueueFamilyProperties> queueFamilies = nativeAdapter.getQueueFamilyProperties();

    // we just need one priority bit, since we are only allocating one queue for both graphics and compute. 
    F32 priority = 1.0f;

    for (U32 i = 0; i < queueFamilies.size(); ++i) {
        VkDeviceQueueCreateInfo queueInfo       = { };
        VkQueueFamilyProperties queueFamProps   = queueFamilies[i];
        B32 shouldCreateQueues = false;

        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        if (queueFamProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) {

            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            shouldCreateQueues = true;

        } else if (queueFamProps.queueFlags & VK_QUEUE_COMPUTE_BIT) {

            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            shouldCreateQueues = true;

        }

        if (shouldCreateQueues) {

            queueCreateInfos.push_back(queueInfo);

        }
        
    }
    
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = (U32)queueCreateInfos.size();

    VkResult result = vkCreateDevice(nativeAdapter(), &createInfo, nullptr, &m_device);

    if (result != VK_SUCCESS) {

        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan device!");
        
        return -1;
    }
    
    return 0;
}


void VulkanDevice::destroy()
{
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Device Destroyed.");

    }
}


ErrType VulkanDevice::createSwapchain(GraphicsSwapchain** ppSwapchain, GraphicsContext* pContext,
    const SwapchainCreateDescription* pDesc)
{

    VulkanSwapchain* pSwapchain     = new VulkanSwapchain();
    VulkanContext*   pNativeContext = static_cast<VulkanContext*>(pContext);
 
    ErrType result = pSwapchain->build(m_device, pNativeContext, pDesc);

    if (result != 0) {
        
        R_ERR(R_CHANNEL_VULKAN, "Swapchain failed to create");

        return -1;
    }

    return result;
}
} // Recluse