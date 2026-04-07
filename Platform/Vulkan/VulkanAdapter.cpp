//
#include "VulkanAdapter.hpp"
#include "VulkanDevice.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Serialization/Hasher.hpp"

#include <set>

namespace Recluse {
namespace Vulkan { 

#define R_CASE_TO_STRING(ff) case ff: return #ff; break

R_INTERNAL
const char* toString(LayerFeatureFlag flag)
{
    switch (flag)
    {
        R_CASE_TO_STRING(LayerFeatureFlag_ApiDump);
        R_CASE_TO_STRING(LayerFeatureFlag_DebugMarking);
        R_CASE_TO_STRING(LayerFeatureFlag_DebugValidation);
        R_CASE_TO_STRING(LayerFeatureFlag_GpuDebugValidation);
        R_CASE_TO_STRING(LayerFeatureFlag_GpuWorkgraphs);
        R_CASE_TO_STRING(LayerFeatureFlag_MeshShading);
        R_CASE_TO_STRING(LayerFeatureFlag_Raytracing);
        R_CASE_TO_STRING(LayerFeatureFlag_SamplerFeedback);
        R_CASE_TO_STRING(LayerFeatureFlag_VariableRateShading);
        R_CASE_TO_STRING(LayerFeatureFlag_GpuCrashReporting);
        default:
            R_CASE_TO_STRING(LayerFeatureFlag_None);
    }

    return "";
}

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
    R_ASSERT(pDevice->getAdapter() != NULL);
    const VkPhysicalDeviceProperties& properties = pDevice->getAdapter()->getProperties();
    return properties.limits.minUniformBufferOffsetAlignment;
}


VkDeviceSize VulkanAdapter::obtainMinStorageBufferOffsetAlignment(VulkanDevice* pDevice)
{
    R_ASSERT(pDevice != NULL);
    R_ASSERT(pDevice->getAdapter() != NULL);
    const VkPhysicalDeviceProperties& properties = pDevice->getAdapter()->getProperties();
    return properties.limits.minStorageBufferOffsetAlignment;
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


VkPhysicalDeviceFeatures VulkanAdapter::internalGetPhysicalFeatures()
{
    VkPhysicalDeviceFeatures features = { };
    vkGetPhysicalDeviceFeatures(m_phyDevice, &features);
    return features;    
}


const VkPhysicalDeviceMemoryProperties& VulkanAdapter::getMemoryProperties() const
{
    return m_memoryProperties;
}


const VkPhysicalDeviceFeatures& VulkanAdapter::getFeatures() const
{
    return m_features;
}


VkPhysicalDeviceMemoryProperties2 VulkanAdapter::getMemoryProperties2() const
{
    VkPhysicalDeviceMemoryProperties2 props = { };
    // Be sure to set the structure type when querying for structures from the physical device,
    // This is important to avoid possible headaches.
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2(m_phyDevice, &props);
    return props;
}


PhysicalDeviceFeaturesInfo VulkanAdapter::getFeatures2() const
{
    PhysicalDeviceFeaturesInfo info = { };
    vkGetPhysicalDeviceFeatures2(m_phyDevice, &info.features2);
    return info;
}


U32 VulkanAdapter::constantBufferOffsetAlignmentBytes() const
{
    return static_cast<U32>(m_properties.limits.minUniformBufferOffsetAlignment);
}


VkDeviceSize VulkanAdapter::obtainMinMemoryMapAlignment(VulkanDevice* pdevice)
{
    return pdevice->getAdapter()->getProperties().limits.minMemoryMapAlignment;
}


ResultCode VulkanAdapter::getAdapterInfo(AdapterInfo* out) const
{
    VkPhysicalDeviceProperties properties = getProperties();
    memcpy(out->deviceName, properties.deviceName, 256);
    out->vendorId = properties.vendorID;

    switch (properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_CPU: out->type = AdapterInfo::Type_Cpu; break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: out->type = AdapterInfo::Type_DiscreteGpu; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: out->type = AdapterInfo::Type_IntegratedGpu; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: out->type = AdapterInfo::Type_VirtualGpu; break;
        default: out->type = AdapterInfo::Type_Unknown; break;
    }

    switch (properties.vendorID) 
    {
        case AMD_VENDOR_ID: out->vendorName = "Advanced Micro Devices"; break;

        case INTEL_VENDOR_ID:  out->vendorName = "Intel Corporation"; break;

        case NVIDIA_VENDOR_ID:  out->vendorName = "Nvidia Corporation"; break;

        case MSFT_VENDOR_ID: out->vendorName = "Microsoft"; break;

        case QUALCOMM_VENDOR_ID: out->vendorName = "Qualcomm Technologies"; break;

        default:
            out->vendorName = "Unknown"; break;
    }

    return RecluseResult_Ok;
}


ResultCode VulkanAdapter::createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) 
{
    static U32 deviceIDIncrement = 0;
    R_DEBUG(R_CHANNEL_VULKAN, "Creating device!");
    VulkanDevice* pDevice = new VulkanDevice();
    ResultCode err = pDevice->initialize(this, info, deviceIDIncrement);
    
    if (err != RecluseResult_Ok) 
    {    
        R_ERROR(R_CHANNEL_VULKAN, "Failed to initialize device!");

        delete pDevice;
        return RecluseResult_Failed;
    }

    m_devices.push_back(pDevice);
    *ppDevice = pDevice;

    deviceIDIncrement++;

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


B32 VulkanAdapter::checkSurfaceSupport(U32 familyQueueIndex, VkSurfaceKHR surface) const
{
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_phyDevice, familyQueueIndex, surface, &supported);
    return (B32)supported;
}


U32 VulkanAdapter::findMemoryType(U32 memoryTypeBitsRequirement, ResourceMemoryUsage usage) const
{ 
    VkMemoryPropertyFlags required = 0;
    VkMemoryPropertyFlags preferred = 0;

    switch (usage) 
    {
        case ResourceMemoryUsage_CpuVisible:
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

    // required - optional
    // TODO: Need to find a better way to query extensions. Some of these have dependencies between device and instance extensions.
    //       We could create a config that has an extension, and its dependency, than create some kind of DAG?
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_None, 
        std::vector<const char*>{   "VK_EXT_host_query_reset", "required", // Cpu side query reset
                                    "VK_KHR_maintenance1", "required" }));  // This is required for fixes on vulkan 1.1.0
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_Raytracing, 
        std::vector<const char*>{   "VK_KHR_ray_tracing_pipeline", "required",
                                    "VK_KHR_acceleration_structure", "required",
                                    "VK_KHR_ray_query", "required",
                                    "VK_KHR_spirv_1_4","required",
                                    "VK_KHR_buffer_device_address", "required",
                                    "VK_KHR_deferred_host_operations", "required",
                                    "VK_EXT_descriptor_indexing", "required",
                                    "VK_KHR_device_group", "required",
                                    "VK_KHR_maintenance3", "required",
                                    "VK_KHR_shader_float_controls", "required",}));
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_MeshShading, 
        std::vector<const char*>{   
#ifdef VK_NV_mesh_shader
                                    VK_NV_MESH_SHADER_EXTENSION_NAME, "optional",
#endif
#ifdef VK_EXT_mesh_shader
                                    VK_EXT_MESH_SHADER_EXTENSION_NAME, "required",
#endif
                                    "VK_KHR_spirv_1_4", "required",
                                    "VK_KHR_shader_float_controls", "required"}));
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_SamplerFeedback,
        std::vector<const char*>{   "VK_NV_shader_image_footprint", "required", }));
    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_VariableRateShading,
        std::vector<const char*>{   "VK_KHR_fragment_shading_rate", "required",
                                    "VK_KHR_create_renderpass2", "required",
                                    "VK_KHR_multiview", "required",
                                    "VK_KHR_maintenance2", "required",
                                    "VK_KHR_get_physical_device_properties2", "required" }));


    m_supportedDeviceExtensions.push_back(std::make_tuple(LayerFeatureFlag_GpuCrashReporting,
        std::vector<const char*>{   
#ifdef VK_NV_device_diagnostics_config
                                    VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME, "required",
#endif
#ifdef VK_NV_device_diagnostic_checkpoints
                                    VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME, "optional",
#endif
    }));
    
    m_supportedDeviceExtensionFlags = getInstance()->getRequestedDeviceFeatures();

    // Query all device extensions available for this device.
    for (U32 i = 0; i < m_supportedDeviceExtensions.size(); ++i) 
    {
        B32 found = false;
        for (I32 extI = 0; extI < std::get<1>(m_supportedDeviceExtensions[i]).size(); extI += 2)
        {
            const char* extensionStr = std::get<1>(m_supportedDeviceExtensions[i])[extI];
            const char* requisite = std::get<1>(m_supportedDeviceExtensions[i])[extI + 1];
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

            if (!found && (strncmp("required", requisite, 9) == 0)) 
            {
                R_WARN
                    (
                        R_CHANNEL_VULKAN, 
                        "%s not found for device (id=%d). Removing extension %s", 
                        std::get<1>(m_supportedDeviceExtensions[i])[extI],
                        m_id,
                        toString(std::get<0>(m_supportedDeviceExtensions[i]))
                    );
                m_supportedDeviceExtensionFlags &= ~(std::get<0>(m_supportedDeviceExtensions[i]));
                m_supportedDeviceExtensions.erase(m_supportedDeviceExtensions.begin() + i);
                --i;
                break;
            }
            else if (!found && (strncmp("optional", requisite, 9) == 0))
            {
                // Remove the extension and its requisite from the requested list.
                auto& extensions = std::get<1>(m_supportedDeviceExtensions[i]);
                R_WARN
                    (
                        R_CHANNEL_VULKAN,
                        "%s was not found, but was optional for device (id=%d). Removing this extension only, as it should not interfere with feature %s",
                        extensions[extI],
                        m_id,
                        toString(std::get<0>(m_supportedDeviceExtensions[i]))
                    );
                extensions.erase(extensions.begin() + extI + 1);
                extensions.erase(extensions.begin() + extI);
                extI -= 2;
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
                    // Need to iterate +2, since we now have requisites.
                    for (U32 j = 0; j < std::get<1>(m_supportedDeviceExtensions[i]).size(); j += 2)
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


std::vector<VkPresentModeKHR> VulkanAdapter::getSupportedPresentModes(VkSurfaceKHR surface) const
{
    std::vector<VkPresentModeKHR> modes;
    uint32_t numModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_phyDevice, surface, &numModes, nullptr);
    modes.resize(numModes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_phyDevice, surface, &numModes, modes.data());
    return modes;
}

VkSurfaceCapabilitiesKHR VulkanAdapter::getSurfaceCapabilities(VkSurfaceKHR surface) const
{
    VkSurfaceCapabilitiesKHR capabilities = { };
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_phyDevice, surface, &capabilities);
    if ( result != VK_SUCCESS )
    {
        R_WARN(R_CHANNEL_VULKAN, "Unable to query surface capabilities! Surface window might either be minimized, or destroyed.");
    }
    return capabilities;
}


Bool VulkanAdapter::checkSupportsDeviceExtension(const char* ext)
{
    std::vector<VkExtensionProperties> deviceExtensions = getDeviceExtensionProperties();
    for (U32 i = 0; i < deviceExtensions.size(); ++i)
    {
        // We found the right extension
        if (strcmp(ext, deviceExtensions[i].extensionName) == 0)
        {
            return true;
        }
    }
    // No supported extension found, return false.
    return false;
}
} // Vulkan
} // Recluse 