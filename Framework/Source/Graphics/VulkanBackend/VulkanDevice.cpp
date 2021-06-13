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

    for (U32 i = 0; i < queueFamilies.size(); ++i) {
        //queueFamilies[i];
        //VkDeviceQueueCreateInfo deviceInfo;
        
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