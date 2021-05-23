// Recluse
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "Core/Messaging.hpp"
#include <vector>

namespace Recluse {


static std::vector<const char*> loadExtensions()
{
    std::vector<const char*> extensions = { 
#if defined(RECLUSE_WINDOWS)
    "VK_KHR_win32_surface",
#endif
    "VK_KHR_get_physical_device_properties2",
    "VK_KHR_get_memory_requirements2"
    };

    for (U32 i = 0; i < extensions.size(); ++i) {
        U32 count = 0;
        std::vector<VkExtensionProperties> properties;

        VkResult result = vkEnumerateInstanceExtensionProperties(extensions[i], &count, nullptr);

        if (result == VK_SUCCESS) {
            properties.resize(count);
            vkEnumerateInstanceExtensionProperties(extensions[i], &count, properties.data());    

            R_DEBUG("Vulkan", "Found extension %s : version %d ", 
                properties[0].extensionName, properties[0].specVersion);

        } else {
            // Not supported.

            R_ERR("Vulkan", "Extension %s not supported.", extensions[i]);

            extensions.erase(extensions.begin() + i);
            --i;
        }
    }
    return extensions;
}


static std::vector<const char*> loadLayers(EnableLayerFlags flags)
{
    std::vector<VkLayerProperties> layerProperties;
    U32 count = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&count, nullptr);
    layerProperties.resize(count);
    result = vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
    std::vector<const char*> desiredLayers = { };
    if (flags & LAYER_FEATURE_DEBUG_VALIDATION_BIT) {
        desiredLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    if (flags & LAYER_FEATURE_API_DUMP_BIT) {
        desiredLayers.push_back("VK_LAYER_LUNARG_api_dump");
    }

    // Seach and remove any layers that aren't available in the instance.
    for (I32 i = 0; i < desiredLayers.size(); ++i) {
        B32 found = false;
        for (U32 j = 0; j < layerProperties.size(); ++j) {
            VkLayerProperties properties = layerProperties[j];
            // If 
            if (strcmp(desiredLayers[i], properties.layerName) == 0) {
                found = true;    
        
                R_DEBUG("Vulkan", "Found layer %s : version %d", properties.layerName, properties.specVersion);

                break;
            }
        }
        if (!found) {
            // Remove and decrement search index by 1.

            R_ERR("Vulkan", "%s was not found.", desiredLayers[i]);

            desiredLayers.erase(desiredLayers.begin() + i);
            --i;
        }
    }
    return desiredLayers;
}


ErrType VulkanContext::onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags)
{
    std::vector<const char*> extensions = loadExtensions();
    std::vector<const char*> layers = loadLayers(flags);

    VkApplicationInfo nativeAppInfo     = { };    
    VkInstanceCreateInfo createInfo     = { };

    m_appName                           = appInfo.appName;
    m_engineName                        = appInfo.engineName;

    nativeAppInfo.sType                 = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    nativeAppInfo.apiVersion =          VK_API_VERSION_1_0;
    nativeAppInfo.pApplicationName      = appInfo.appName;
    nativeAppInfo.pEngineName           = appInfo.engineName;
    nativeAppInfo.engineVersion         = VK_MAKE_VERSION(appInfo.engineMajor, appInfo.engineMinor, appInfo.enginePatch);
    nativeAppInfo.applicationVersion    = VK_MAKE_VERSION(appInfo.appMajor, appInfo.appMinor, appInfo.appPatch);
    nativeAppInfo.pNext = nullptr;
    
    createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo         = &nativeAppInfo;
    createInfo.enabledLayerCount        = (U32)layers.size();
    createInfo.ppEnabledLayerNames      = layers.data();
    createInfo.enabledExtensionCount    = (U32)extensions.size();
    createInfo.ppEnabledExtensionNames  = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        nullify();
    }

    R_DEBUG("Vulkan", "Application: %s\nEngine: %s", m_appName.c_str(), m_engineName.c_str());

    m_engineVersion = nativeAppInfo.engineVersion;
    m_appVersion = nativeAppInfo.applicationVersion;

    return 0;
}


void VulkanContext::nullify()
{
    m_instance = VK_NULL_HANDLE;
}


void VulkanContext::onDestroy()
{
    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        nullify();
    }
}


PFN_vkVoidFunction VulkanContext::getProcAddr(const char* funcName)
{
    return vkGetInstanceProcAddr(m_instance, funcName);
}


void VulkanContext::queryGraphicsAdapters()
{
    std::vector<VulkanAdapter> devices = VulkanAdapter::getAvailablePhysicalDevices(*this);
    std::vector<GraphicsAdapter*> adapters(devices.size());

    for (U32 i = 0; i < adapters.size(); ++i) {
        adapters[i] = new VulkanAdapter(devices[i]);
    }

    m_graphicsAdapters = adapters;
}


void VulkanContext::freeGraphicsAdapters()
{
    for (U32 i = 0; i < m_graphicsAdapters.size(); ++i) {
        delete m_graphicsAdapters[i];
    }
}
} // Recluse