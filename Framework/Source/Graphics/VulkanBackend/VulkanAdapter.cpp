//
#include "VulkanAdapter.hpp"
#include "VulkanDevice.hpp"
#include "Core/Messaging.hpp"

namespace Recluse {


std::vector<VulkanAdapter> VulkanAdapter::getAvailablePhysicalDevices(const VulkanContext& ctx)
{
    std::vector<VulkanAdapter> physicalDevices;
    U32 count = 0;
    VkResult result = vkEnumeratePhysicalDevices(ctx(), &count, nullptr);
    
    if (count == 0) {

        R_ERR(R_CHANNEL_VULKAN, "No physical devices that support vulkan!");

        return physicalDevices;
    }

    if (result == VK_SUCCESS) {
        std::vector<VkPhysicalDevice> devices(count);

        physicalDevices.resize(count);

        vkEnumeratePhysicalDevices(ctx(), &count, devices.data());

        R_DEBUG(R_CHANNEL_VULKAN, 
            ((count > 1) ? "There are %d vulkan devices." : "There is %d vulkan device."), count);        

        for (U32 i = 0; i < count; ++i) {
            VulkanAdapter device;
            device.m_phyDevice = devices[i];
            physicalDevices[i] = device;
        }
    }

    return physicalDevices;
}


VkPhysicalDeviceProperties VulkanAdapter::getProperties() const
{
    VkPhysicalDeviceProperties props = { };
    vkGetPhysicalDeviceProperties(m_phyDevice, &props);
    return props;
}


VkPhysicalDeviceMemoryProperties VulkanAdapter::getMemoryProperties() const
{
    VkPhysicalDeviceMemoryProperties props = { };
    vkGetPhysicalDeviceMemoryProperties(m_phyDevice, &props);
    return props;
}


VkPhysicalDeviceFeatures VulkanAdapter::getFeatures() const
{
    VkPhysicalDeviceFeatures features = { };
    vkGetPhysicalDeviceFeatures(m_phyDevice, &features);
    return features;
}


VkPhysicalDeviceMemoryProperties2 VulkanAdapter::getMemoryProperties2() const
{
    VkPhysicalDeviceMemoryProperties2 props = { };
    vkGetPhysicalDeviceMemoryProperties2(m_phyDevice, &props);
    return props;
}


VkPhysicalDeviceFeatures2 VulkanAdapter::getFeatures2() const
{
    VkPhysicalDeviceFeatures2 features = { };
    vkGetPhysicalDeviceFeatures2(m_phyDevice, &features);
    return features;
}


ErrType VulkanAdapter::getAdapterInfo(AdapterInfo* out) const
{
    VkPhysicalDeviceProperties properties = getProperties();
    memcpy(out->deviceName, properties.deviceName, 256);
    out->vendorId = properties.vendorID;

    switch (properties.vendorID) {
        case AMD_VENDOR_ID: out->vendor = VENDOR_AMD; break;
        case INTEL_VENDOR_ID:  out->vendor = VENDOR_INTEL; break;
        case NVIDIA_VENDOR_ID:  out->vendor = VENDOR_NVIDIA; break;
        default:
            out->vendor = VENDOR_UNKNOWN; break;
    }    

    return 0;
}


ErrType VulkanAdapter::createDevice(DeviceCreateInfo* info, GraphicsDevice** ppDevice) 
{

    R_DEBUG(R_CHANNEL_VULKAN, "Creating device!");

    VulkanDevice* pDevice = new VulkanDevice();
    ErrType err = pDevice->initialize(*this, info);
    
    if (err != 0) {
        
        R_ERR(R_CHANNEL_VULKAN, "Failed to initialize device!");

        delete pDevice;
        return -1;
    }

    m_devices.push_back(pDevice);
    *ppDevice = pDevice;

    return 0;
}


ErrType VulkanAdapter::destroyDevice(GraphicsDevice* pDevice)
{
    for (auto& iter = m_devices.begin(); iter != m_devices.end(); ++iter) {
        if (*iter == pDevice) {    

            R_DEBUG(R_CHANNEL_VULKAN, "Destroying device!");

            (*iter)->destroy();

            delete *iter;
            m_devices.erase(iter);

            return 0;
        }
    }

    R_ERR(R_CHANNEL_VULKAN, "Device does not belong to this adapter!");   
 
    return 0;
}


std::vector<VkQueueFamilyProperties> VulkanAdapter::getQueueFamilyProperties() const
{
    std::vector<VkQueueFamilyProperties> properties = { };
    U32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_phyDevice, &count, nullptr);

    if (count == 0) {
    
        R_ERR(R_CHANNEL_VULKAN, "No queue families reported by the driver!");
        
        return properties;
    }

    properties.resize(count);
    
    vkGetPhysicalDeviceQueueFamilyProperties(m_phyDevice, &count, properties.data());

    return properties;
}


VulkanAdapter::~VulkanAdapter()
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying Adapter...");

    if (!m_devices.empty()) {
        R_ERR(R_CHANNEL_VULKAN, "One or more devices exist for this adapter!");
    }
}
} // Recluse 