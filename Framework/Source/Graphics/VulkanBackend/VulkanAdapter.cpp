//
#include "VulkanAdapter.hpp"
#include "VulkanDevice.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include <set>

namespace Recluse {


std::vector<VulkanAdapter> VulkanAdapter::getAvailablePhysicalDevices(VulkanInstance* ctx)
{
    std::vector<VulkanAdapter> physicalDevices;
    U32 count = 0;
    VkResult result = vkEnumeratePhysicalDevices(ctx->get(), &count, nullptr);
    
    if (count == 0) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "No physical devices that support vulkan!");

        return physicalDevices;
    }

    if (result == VK_SUCCESS) 
    {
        std::vector<VkPhysicalDevice> devices(count);

        physicalDevices.resize(count);

        vkEnumeratePhysicalDevices(ctx->get(), &count, devices.data());

        R_DEBUG(R_CHANNEL_VULKAN, 
            ((count != 1) ? "There are %d vulkan devices." : "There is %d vulkan device."), count);        

        for (U32 i = 0; i < count; ++i) 
        {
            VulkanAdapter device(i);
            device.m_phyDevice = devices[i];
            device.m_instance = ctx;
            device.checkAvailableDeviceExtensions();
            physicalDevices[i] = std::move(device);
        }
    }

    return physicalDevices;
}


VkDeviceSize VulkanAdapter::obtainMinUniformBufferOffsetAlignment(VulkanDevice* pDevice)
{
    R_ASSERT(pDevice != NULL);
    const VkPhysicalDeviceProperties& properties = pDevice->getAdapter()->getProperties();
    return properties.limits.minUniformBufferOffsetAlignment;
}


VkPhysicalDeviceProperties VulkanAdapter::internalGetPhysicalProperties()
{
    VkPhysicalDeviceProperties props = { };
    vkGetPhysicalDeviceProperties(m_phyDevice, &props);
    return props;
}

const VkPhysicalDeviceProperties& VulkanAdapter::getProperties() const
{
    return m_properties;
}


VkPhysicalDeviceMemoryProperties VulkanAdapter::internalGetPhysicalMemoryProperties()
{
    VkPhysicalDeviceMemoryProperties props = { };
    vkGetPhysicalDeviceMemoryProperties(m_phyDevice, &props);
    return props;
}


const VkPhysicalDeviceMemoryProperties& VulkanAdapter::getMemoryProperties() const
{
    return m_memoryProperties;
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


ResultCode VulkanAdapter::getAdapterInfo(AdapterInfo* out) const
{
    VkPhysicalDeviceProperties properties = getProperties();
    memcpy(out->deviceName, properties.deviceName, 256);
    out->vendorId = properties.vendorID;

    switch (properties.vendorID) 
    {
        case AMD_VENDOR_ID: out->vendorName = "Advanced Micro Devices"; break;

        case INTEL_VENDOR_ID:  out->vendorName = "Intel Corporation"; break;

        case NVIDIA_VENDOR_ID:  out->vendorName = "Nvidia Corporation"; break;

        case MSFT_VENDOR_ID: out->vendorName = "Microsoft"; break;

        default:
            out->vendorName = "Unknown"; break;
    }    

    return RecluseResult_Ok;
}


ResultCode VulkanAdapter::createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) 
{

    R_DEBUG(R_CHANNEL_VULKAN, "Creating device!");

    VulkanDevice* pDevice = new VulkanDevice();
    ResultCode err = pDevice->initialize(this, info);
    
    if (err != RecluseResult_Ok) 
    {    
        R_ERROR(R_CHANNEL_VULKAN, "Failed to initialize device!");

        delete pDevice;
        return RecluseResult_Failed;
    }

    m_devices.push_back(pDevice);
    *ppDevice = pDevice;

    return RecluseResult_Ok;
}


ResultCode VulkanAdapter::destroyDevice(GraphicsDevice* pDevice)
{
    VulkanInstance* pVc = m_instance;

    for (auto& iter = m_devices.begin(); iter != m_devices.end(); ++iter) 
    {
        if (*iter == pDevice) 
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Destroying device!");

            (*iter)->release(pVc->get());

            delete *iter;
            m_devices.erase(iter);

            return RecluseResult_Ok;
        }
    }

    R_ERROR(R_CHANNEL_VULKAN, "Device does not belong to this adapter!");   
 
    return RecluseResult_Ok;
}


std::vector<VkQueueFamilyProperties> VulkanAdapter::getQueueFamilyProperties() const
{
    std::vector<VkQueueFamilyProperties> properties = { };
    U32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_phyDevice, &count, nullptr);

    if (count == 0) 
    { 
        R_ERROR(R_CHANNEL_VULKAN, "No queue families reported by the driver!");
        
        return properties;
    }

    properties.resize(count);
    
    vkGetPhysicalDeviceQueueFamilyProperties(m_phyDevice, &count, properties.data());

    return properties;
}


VulkanAdapter::~VulkanAdapter()
{
    if (m_phyDevice && (!m_devices.empty())) 
    {
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


U32 VulkanAdapter::findMemoryType(U32 memoryTypeBitsRequirement, ResourceMemoryUsage usage) const
{ 
    VkMemoryPropertyFlags required = 0;
    VkMemoryPropertyFlags preferred = 0;

    switch (usage) 
    {
        case ResourceMemoryUsage_CpuOnly:
        {
            required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            preferred |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        } break;

        case ResourceMemoryUsage_GpuOnly:
        {
            required |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } break;

        case ResourceMemoryUsage_CpuToGpu:
        {
            required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } break;

        case ResourceMemoryUsage_GpuToCpu:
        {
            required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            preferred |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        } break;

        default:
            break;
    }

    VkPhysicalDeviceMemoryProperties memoryProperties = getMemoryProperties();

    for (U32 memoryIndex = 0; memoryIndex < memoryProperties.memoryTypeCount; ++memoryIndex) 
    {
        const U32 memoryTypeBits = (1 << memoryIndex);
        const VkMemoryPropertyFlags properties = memoryProperties.memoryTypes[memoryIndex].propertyFlags;
        const Bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;
        const Bool hasRequiredProperties = ((properties & required) == required);
        const Bool hasPreferredProperties = ((properties & preferred) == preferred) && hasRequiredProperties;

        if (isRequiredMemoryType && (hasPreferredProperties || hasRequiredProperties))
        {
            return memoryIndex;
        }
    }

    // Couldn't find the requirements we were looking for.
    return 0xffffffff;
}


VkFormatProperties VulkanAdapter::getFormatProperties(VkFormat format) const
{
    VkFormatProperties props = { };

    vkGetPhysicalDeviceFormatProperties(m_phyDevice, format, &props);
    
    return props;
}


std::vector<VkSurfaceFormatKHR> VulkanAdapter::getSurfaceFormats(VkSurfaceKHR surface)
{
    R_ASSERT(m_phyDevice != NULL);

    U32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_phyDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_phyDevice, surface, &formatCount, formats.data());
    return formats;
}


void VulkanAdapter::checkAvailableDeviceExtensions()
{
    std::vector<VkExtensionProperties> deviceExtensions = getDeviceExtensionProperties();

    // TODO: Need to find a better way to query extensions. Some of these have dependencies between device and instance extensions.
    //       We could create a config that has an extension, and its dependency, than create some kind of DAG?
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_Raytracing, 
        std::vector<const char*>{   "VK_KHR_ray_tracing_pipeline", 
                                    "VK_KHR_acceleration_structure", 
                                    "VK_KHR_ray_query", 
                                    "VK_KHR_spirv_1_4",
                                    "VK_KHR_buffer_device_address",
                                    "VK_KHR_deferred_host_operations",
                                    "VK_EXT_descriptor_indexing",
                                    "VK_KHR_device_group",
                                    "VK_KHR_maintenance3",
                                    "VK_KHR_shader_float_controls"}));
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_MeshShading, 
        std::vector<const char*>{   "VK_EXT_mesh_shader", 
                                    "VK_KHR_spirv_1_4",
                                    "VK_KHR_shader_float_controls"}));
    
    m_supportedDeviceExtensionFlags = LayerFeatureFlag_MeshShading | LayerFeatureFlag_Raytracing;

    // Query all device extensions available for this device.
    for (U32 i = 0; i < m_supportedDeviceExtensions.size(); ++i) 
    {
        B32 found = false;
        for (U32 extI = 0; extI < std::get<1>(m_supportedDeviceExtensions[i]).size(); ++extI)
        {
            const char* extensionStr = std::get<1>(m_supportedDeviceExtensions[i])[extI];
            for (U32 j = 0; j < deviceExtensions.size(); ++j) 
            { 
                if (strcmp(deviceExtensions[j].extensionName, extensionStr) == 0) 
                {
                    R_DEBUG
                        (
                            R_CHANNEL_VULKAN, 
                            "Found %s Spec Version: %d on device (id=%d)", 
                            deviceExtensions[j].extensionName,
                            deviceExtensions[j].specVersion,
                            m_id
                        );

                    found = true;
                    break;
                }
    
            }

            if (!found) 
            {
                R_WARN
                    (
                        R_CHANNEL_VULKAN, 
                        "%s not found for device (id=%d). Removing extension.", 
                        std::get<1>(m_supportedDeviceExtensions[i])[extI],
                        m_id
                    );
                m_supportedDeviceExtensionFlags &= ~(std::get<0>(m_supportedDeviceExtensions[i]));
                m_supportedDeviceExtensions.erase(m_supportedDeviceExtensions.begin() + i);
                --i;
                break;
            }
        }
    }
}


std::vector<const char*> VulkanAdapter::queryAvailableDeviceExtensions(LayerFeatureFlags requested) const
{

    struct Comp
    {
        bool operator()(const char* p0, const char* p1) const
        {
            return (strcmp(p0, p1) > 0);
        }
    };

    std::set<const char*, Comp> supportedExtensions;
    std::vector<const char*> extensions;
    for (U32 bit = 1; bit != 0; bit <<= 1)
    {
        if (bit & requested)
        {
            for (U32 i = 0; i < m_supportedDeviceExtensions.size(); ++i)
            {
                if (bit & std::get<0>(m_supportedDeviceExtensions[i]))
                {
                    for (U32 j = 0; j < std::get<1>(m_supportedDeviceExtensions[i]).size(); ++j)
                    {
                        supportedExtensions.insert(std::get<1>(m_supportedDeviceExtensions[i])[j]);
                    }
                }
            }
        }
    }

    for (const char* str : supportedExtensions)
    {
        extensions.push_back(str);
    }

    return extensions;
}
} // Recluse 