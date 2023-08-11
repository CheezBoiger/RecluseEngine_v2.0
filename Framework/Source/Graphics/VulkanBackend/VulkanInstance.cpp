// Recluse
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanShaderCache.hpp"
#include "Recluse/Messaging.hpp"
#include <vector>

namespace Recluse {


static VKAPI_ATTR VkBool32 VKAPI_CALL recluseDebugCallback
    (
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        U64 obj,
        size_t location,
        I32 code,
        const char* layerPrefix,
        const char* msg,
        void* usrData
    )
{
    R_DEBUG(R_CHANNEL_VULKAN, "Validation layer: %s\n", msg);
    R_ASSERT_FORMAT(!(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT), "Vulkan Error.");
    return VK_FALSE;
}


void VulkanInstance::setDebugCallback()
{
    VkDebugReportCallbackCreateInfoEXT ci = { };
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    ci.pfnCallback = recluseDebugCallback;
    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
    if (!vkCreateDebugReportCallbackEXT) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to initialize our vulkan reporting.");
        return;
    }

    VkResult result = vkCreateDebugReportCallbackEXT(m_instance, &ci, nullptr, &m_debugReportCallback);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create debug report callback!");
    }
}


static std::vector<const char*> loadExtensions(LayerFeatureFlags flags, std::vector<LayerFeatureFlag>& wantedExtBits)
{
    std::vector<const char*> extensions = 
        { 
            VK_KHR_SURFACE_EXTENSION_NAME
#if defined(RECLUSE_WINDOWS)
            , VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
            , "VK_KHR_get_physical_device_properties2"
        };

    if (flags & LayerFeatureFlag_DebugValidation) 
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        wantedExtBits.push_back(LayerFeatureFlag_DebugValidation);    
    }

    if (flags & (LayerFeatureFlag_Raytracing | LayerFeatureFlag_MeshShading))
    {
        extensions.push_back("VK_KHR_device_group_creation");
    }

    if (flags & LayerFeatureFlag_DebugMarking)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        wantedExtBits.push_back(LayerFeatureFlag_DebugMarking);
    }

    return extensions;
}


static void checkForValidExtensions(std::vector<const char*>& wantedExtensions, std::vector<LayerFeatureFlag>& wantedExtBits)
{
    U32 count = 0;
    std::vector<VkExtensionProperties> properties;

    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to query for vulkan instance extensions!");
        return;
    }

    properties.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());    


    for (U32 i = 0; i < wantedExtensions.size(); ++i) 
    {
        B32 foundExtension = false;

        for (U32 j = 0; j < properties.size(); ++j) 
        {
            if (strcmp(properties[j].extensionName, wantedExtensions[i]) == 0) 
            {
                R_DEBUG
                    (
                        R_CHANNEL_VULKAN, 
                        "Found extension %s : version %d ", 
                        properties[j].extensionName, 
                        properties[j].specVersion
                    );

                foundExtension = true;
                break;
            } 
        }

        if (!foundExtension) 
        {
            // Not supported.
            R_WARN(R_CHANNEL_VULKAN, "Extension %s not supported.", wantedExtensions[i]);
            wantedExtensions.erase(wantedExtensions.begin() + i);
            wantedExtBits.erase(wantedExtBits.begin() + i);
            --i;
        }
    }
}


static std::vector<const char*> loadLayers(LayerFeatureFlags flags, std::vector<LayerFeatureFlag>& wantedLayers)
{
    std::vector<const char*> desiredLayers = { };

    if (flags & LayerFeatureFlag_DebugValidation) 
    {
        desiredLayers.push_back("VK_LAYER_KHRONOS_validation");
        wantedLayers.push_back(LayerFeatureFlag_DebugValidation);
    }

    if (flags & LayerFeatureFlag_ApiDump) 
    {
        desiredLayers.push_back("VK_LAYER_LUNARG_api_dump");
        wantedLayers.push_back(LayerFeatureFlag_ApiDump);
    }

    return desiredLayers;
}


void checkForValidLayers(std::vector<const char*>& wantedLayers, std::vector<LayerFeatureFlag>& wantedBits)
{
    std::vector<VkLayerProperties> layerProperties;
    U32 count = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&count, nullptr);

    layerProperties.resize(count);
    result = vkEnumerateInstanceLayerProperties(&count, layerProperties.data());

    // Seach and remove any layers that aren't available in the instance.
    for (I32 i = 0; i < wantedLayers.size(); ++i) 
    {
        B32 found = false;
        for (U32 j = 0; j < layerProperties.size(); ++j) 
        {
            VkLayerProperties properties = layerProperties[j];

            // If 
            if (strcmp(wantedLayers[i], properties.layerName) == 0) 
            {
                found = true;    
                R_DEBUG(R_CHANNEL_VULKAN, "Found layer %s : version %d", properties.layerName, properties.specVersion);
                break;
            }
        }

        if (!found) 
        {
            // Remove and decrement search index by 1.
            R_WARN(R_CHANNEL_VULKAN, "%s was not found.", wantedLayers[i]);
            wantedLayers.erase(wantedLayers.begin() + i);
            wantedBits.erase(wantedBits.begin() + i);
            --i;
        }
    }
}


ResultCode VulkanInstance::onInitialize(const ApplicationInfo& appInfo, LayerFeatureFlags flags)
{
    std::vector<LayerFeatureFlag> wantedLayerBits   = { };
    std::vector<LayerFeatureFlag> wantedExtBits     = { };
    std::vector<const char*> extensions             = loadExtensions(flags, wantedExtBits);
    std::vector<const char*> layers                 = loadLayers(flags, wantedLayerBits);

    checkForValidExtensions(extensions, wantedExtBits);
    checkForValidLayers(layers, wantedLayerBits);
    
    VkApplicationInfo nativeAppInfo         = { };    
    VkInstanceCreateInfo createInfo         = { };

    m_debugReportCallback                   = VK_NULL_HANDLE;
    m_appName                               = appInfo.appName;
    m_engineName                            = appInfo.engineName;

    nativeAppInfo.sType                     = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    nativeAppInfo.apiVersion                = VK_MAKE_VERSION(1, 1, 0); // We will target Vulkan v1.1.0
    nativeAppInfo.pApplicationName          = appInfo.appName;
    nativeAppInfo.pEngineName               = appInfo.engineName;
    nativeAppInfo.engineVersion             = VK_MAKE_VERSION(appInfo.engineMajor, appInfo.engineMinor, appInfo.enginePatch);
    nativeAppInfo.applicationVersion        = VK_MAKE_VERSION(appInfo.appMajor, appInfo.appMinor, appInfo.appPatch);
    nativeAppInfo.pNext = nullptr;
    
    createInfo.sType                        = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo             = &nativeAppInfo;
    createInfo.enabledLayerCount            = (U32)layers.size();
    createInfo.ppEnabledLayerNames          = layers.data();
    createInfo.enabledExtensionCount        = (U32)extensions.size();
    createInfo.ppEnabledExtensionNames      = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    
    if (result != VK_SUCCESS) 
    {
        nullify();
    }

    R_DEBUG(R_CHANNEL_VULKAN, "Application: %s\nEngine: %s", m_appName.c_str(), m_engineName.c_str());

    if (flags & LayerFeatureFlag_DebugValidation) 
    {
        setDebugCallback();
    }

    for (LayerFeatureFlag featureBit : wantedLayerBits)
    {
        m_enabledLayers |= featureBit;
    }
    
    for (LayerFeatureFlag featureBit : wantedExtBits)
    {
        m_enabledLayers |= featureBit;
    }

    m_engineVersion = nativeAppInfo.engineVersion;
    m_appVersion    = nativeAppInfo.applicationVersion;

    setRequestedDeviceExtensions(flags);

    queryFunctions();

    return 0;
}


void VulkanInstance::nullify()
{
    m_instance = VK_NULL_HANDLE;
}


void VulkanInstance::onDestroy()
{

    destroyDebugCallback();

    if (m_instance) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying context...");

        vkDestroyInstance(m_instance, nullptr);
        nullify();

        R_DEBUG(R_CHANNEL_VULKAN, "Successfully destroyed context!");
    }
}


PFN_vkVoidFunction VulkanInstance::getProcAddr(const char* funcName)
{
    return vkGetInstanceProcAddr(m_instance, funcName);
}


void VulkanInstance::queryGraphicsAdapters()
{
    std::vector<VulkanAdapter> devices = VulkanAdapter::getAvailablePhysicalDevices(this);
    std::vector<GraphicsAdapter*> adapters(devices.size());

    for (U32 i = 0; i < adapters.size(); ++i) 
    {
        adapters[i] = new VulkanAdapter(std::move(devices[i]));
    }

    m_graphicsAdapters = adapters;
}


void VulkanInstance::freeGraphicsAdapters()
{
    for (U32 i = 0; i < m_graphicsAdapters.size(); ++i) 
    {
        delete m_graphicsAdapters[i];
    }
}


void VulkanInstance::destroyDebugCallback()
{
    if (m_debugReportCallback) 
    {
        auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");

        if (vkDestroyDebugReportCallbackEXT) 
        {
            vkDestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
            m_debugReportCallback = VK_NULL_HANDLE;
        }
    }
}


void VulkanInstance::setRequestedDeviceExtensions(LayerFeatureFlags flags)
{
    if (flags & LayerFeatureFlag_MeshShading)
        m_requestedDeviceFeatures |= LayerFeatureFlag_MeshShading;
    if (flags & LayerFeatureFlag_Raytracing)
        m_requestedDeviceFeatures |= LayerFeatureFlag_Raytracing;
}


void VulkanInstance::queryFunctions()
{
    if (supportsDebugMarking())
    {
        if (!pfn_vkSetDebugUtilsObjectNameEXT)
            pfn_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)getProcAddr("vkSetDebugUtilsObjectNameEXT");
        if (!pfn_vkSetDebugUtilsObjectTagEXT)
            pfn_vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)getProcAddr("vkSetDebugUtilsObjectTagEXT");
    }
}
} // Recluse