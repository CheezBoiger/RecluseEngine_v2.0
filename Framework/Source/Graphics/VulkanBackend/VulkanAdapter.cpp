//
#include "VulkanAdapter.hpp"
#include "VulkanDevice.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


std::vector<VulkanAdapter> VulkanAdapter::getAvailablePhysicalDevices(VulkanInstance* ctx)
{
    std::vector<VulkanAdapter> physicalDevices;
    U32 count = 0;
    VkResult result = vkEnumeratePhysicalDevices(ctx->get(), &count, nullptr);
    
    if (count == 0) {

        R_ERR(R_CHANNEL_VULKAN, "No physical devices that support vulkan!");

        return physicalDevices;
    }

    if (result == VK_SUCCESS) {
        std::vector<VkPhysicalDevice> devices(count);

        physicalDevices.resize(count);

        vkEnumeratePhysicalDevices(ctx->get(), &count, devices.data());

        R_DEBUG(R_CHANNEL_VULKAN, 
            ((count > 1) ? "There are %d vulkan devices." : "There is %d vulkan device."), count);        

        for (U32 i = 0; i < count; ++i) {
            VulkanAdapter device;
            device.m_phyDevice = devices[i];
            device.m_instance = ctx;
            physicalDevices[i] = std::move(device);
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
        case AMD_VENDOR_ID: out->vendorName = "Advanced Micro Devices"; break;
        case INTEL_VENDOR_ID:  out->vendorName = "Intel Corporation"; break;
        case NVIDIA_VENDOR_ID:  out->vendorName = "Nvidia Corporation"; break;
        case MSFT_VENDOR_ID: out->vendorName = "Microsoft"; break;
        default:
            out->vendorName = "Unknown"; break;
    }    

    return REC_RESULT_OK;
}


ErrType VulkanAdapter::createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) 
{

    R_DEBUG(R_CHANNEL_VULKAN, "Creating device!");

    VulkanDevice* pDevice = new VulkanDevice();
    ErrType err = pDevice->initialize(this, info);
    
    if (err != REC_RESULT_OK) {
        
        R_ERR(R_CHANNEL_VULKAN, "Failed to initialize device!");

        delete pDevice;
        return REC_RESULT_FAILED;
    }

    m_devices.push_back(pDevice);
    *ppDevice = pDevice;

    return REC_RESULT_OK;
}


ErrType VulkanAdapter::destroyDevice(GraphicsDevice* pDevice)
{
    VulkanInstance* pVc = m_instance;

    for (auto& iter = m_devices.begin(); iter != m_devices.end(); ++iter) {
        if (*iter == pDevice) {    

            R_DEBUG(R_CHANNEL_VULKAN, "Destroying device!");

            (*iter)->destroy(pVc->get());

            delete *iter;
            m_devices.erase(iter);

            return REC_RESULT_OK;
        }
    }

    R_ERR(R_CHANNEL_VULKAN, "Device does not belong to this adapter!");   
 
    return REC_RESULT_OK;
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
    if (m_phyDevice && (!m_devices.empty())) {

        R_WARN(R_CHANNEL_VULKAN, "One or more devices exist for this adapter, prior to its handle destruction!");

    }
}


std::vector<VkExtensionProperties> VulkanAdapter::getDeviceExtensionProperties() const
{
    std::vector<VkExtensionProperties> props;
    U32 count = 0;
    vkEnumerateDeviceExtensionProperties(m_phyDevice, nullptr, &count, nullptr);    
    props.resize(count);
    vkEnumerateDeviceExtensionProperties(m_phyDevice, nullptr, &count, props.data());
    return props;
}


B32 VulkanAdapter::checkSurfaceSupport(U32 queueIndex, VkSurfaceKHR surface) const
{
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_phyDevice, queueIndex, surface, &supported);
    return (B32)supported;
}


U32 VulkanAdapter::findMemoryType(U32 filter, ResourceMemoryUsage usage) const
{ 
    VkMemoryPropertyFlags required = 0;
    VkMemoryPropertyFlags preferred = 0;
    U32 index = 0xffffffff;

    switch (usage) {
    case RESOURCE_MEMORY_USAGE_CPU_ONLY:
    {
        required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        preferred |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    } break;
    case RESOURCE_MEMORY_USAGE_GPU_ONLY:
    {
        required |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    } break;
    case RESOURCE_MEMORY_USAGE_CPU_TO_GPU:
    {
        required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    } break;
    case RESOURCE_MEMORY_USAGE_GPU_TO_CPU:
    {
        required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        preferred |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    } break;
    default:
        break;
    };

    VkPhysicalDeviceMemoryProperties memoryProperties = getMemoryProperties();

    for (U32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {

        const VkMemoryPropertyFlags propFlags = memoryProperties.memoryTypes[i].propertyFlags;

        if ((filter & (1 << i))) {

            if ((propFlags & required) == required && ((propFlags & preferred) == preferred)) {

                index = i;
                break;

            } else if ((propFlags & required) == required) {

                index = i;
                break;

            }
        }
    }

    return index;
}


VkFormatProperties VulkanAdapter::getFormatProperties(VkFormat format) const
{
    VkFormatProperties props = { };

    vkGetPhysicalDeviceFormatProperties(m_phyDevice, format, &props);
    
    return props;
}
} // Recluse 