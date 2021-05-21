//
#include "VulkanDevice.hpp"

#include "Core/Messaging.hpp"


namespace Recluse {


std::vector<VulkanPhysicalDevice> VulkanPhysicalDevice::getAvailablePhysicalDevices(const VulkanContext& ctx)
{
    std::vector<VulkanPhysicalDevice> physicalDevices;
    U32 count = 0;
    VkResult result = vkEnumeratePhysicalDevices(ctx(), &count, nullptr);
    
    if (count == 0) {

        R_ERR("Vulkan", "No physical devices that support vulkan!");

        return physicalDevices;
    }

    if (result == VK_SUCCESS) {
        std::vector<VkPhysicalDevice> devices(count);

        physicalDevices.resize(count);

        vkEnumeratePhysicalDevices(ctx(), &count, devices.data());

        R_DEBUG("Vulkan", "There are %d vulkan devices.", count);        

        for (U32 i = 0; i < count; ++i) {
            VulkanPhysicalDevice device;
            device.m_phyDevice = devices[i];
            physicalDevices[i] = device;
        }
    }

    return physicalDevices;
}


VkPhysicalDeviceProperties VulkanPhysicalDevice::getProperties() const
{
    VkPhysicalDeviceProperties props = { };
    vkGetPhysicalDeviceProperties(m_phyDevice, &props);
    return props;
}


VkPhysicalDeviceMemoryProperties VulkanPhysicalDevice::getMemoryProperties() const
{
    VkPhysicalDeviceMemoryProperties props = { };
    vkGetPhysicalDeviceMemoryProperties(m_phyDevice, &props);
    return props;
}


VkPhysicalDeviceFeatures VulkanPhysicalDevice::getFeatures() const
{
    VkPhysicalDeviceFeatures features = { };
    vkGetPhysicalDeviceFeatures(m_phyDevice, &features);
    return features;
}


VkPhysicalDeviceMemoryProperties2 VulkanPhysicalDevice::getMemoryProperties2() const
{
    VkPhysicalDeviceMemoryProperties2 props = { };
    vkGetPhysicalDeviceMemoryProperties2(m_phyDevice, &props);
    return props;
}


VkPhysicalDeviceFeatures2 VulkanPhysicalDevice::getFeatures2() const
{
    VkPhysicalDeviceFeatures2 features = { };
    vkGetPhysicalDeviceFeatures2(m_phyDevice, &features);
    return features;
}


ErrType VulkanPhysicalDevice::getAdapterInfo(AdapterInfo* out) const
{
    VkPhysicalDeviceProperties properties = getProperties();
    out->deviceName = properties.deviceName;
    out->vendorId = properties.vendorID;

    return 0;
}


ErrType VulkanDevice::initialize(const GraphicsAdapter& adapter)
{
    const VulkanPhysicalDevice& nativeAdapter = static_cast<const VulkanPhysicalDevice&>(adapter);
    VkDeviceCreateInfo createInfo = { };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    VkResult result = vkCreateDevice(nativeAdapter(), &createInfo, nullptr, &m_device);
    
    if (result != VK_SUCCESS) {

        R_ERR("Vulkan", "Failed to create vulkan device!");
        
        return -1;
    }

    return 0;
}


void VulkanDevice::destroy()
{
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        R_DEBUG("Vulkan", "Device Destroyed.");

    }
}
} // Recluse