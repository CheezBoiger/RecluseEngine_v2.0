// Recluse 
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "VulkanCommons.hpp"


namespace Recluse {

class VulkanInstance : public GraphicsInstance 
{
public:
    VulkanInstance() 
        : GraphicsInstance(GraphicsApi_Vulkan) 
        , m_requestedDeviceFeatures(LayerFeatureFlag_None)
        , m_enabledLayers(LayerFeatureFlag_None)
        , m_engineVersion(0)
        , m_appVersion(0)
        , m_instance(VK_NULL_HANDLE) { }

    virtual ~VulkanInstance() { }
    // Initialize the vulkan context.
    ErrType onInitialize(const ApplicationInfo& appInfo, LayerFeatureFlags layerFlags) override;

    // Destroy the vulkan context. Be sure to clean up all resources and device, before
    // destroying vulkan context.
    void onDestroy() override;

    // Getter functions.
    VkInstance get() { return m_instance; }

    VkInstance get() const { return m_instance; }

    VkInstance operator()() { return get(); }

    VkInstance operator()() const { return get(); }

    // Get process address.
    PFN_vkVoidFunction getProcAddr(const char* funcName);

    std::string getAppName() { return m_appName; }
    std::string getEngineName() { return m_engineName; }

    void queryGraphicsAdapters() override;
    void freeGraphicsAdapters() override;

    Bool supportsLayers(LayerFeatureFlags layer) const { return (m_enabledLayers & layer); }
    Bool supportsDebugMarking() const { return supportsLayers(LayerFeatureFlag_DebugMarking); }

    LayerFeatureFlags getRequestedDeviceFeatures() const { return m_requestedDeviceFeatures; }
    
private:

    void nullify();

    void setDebugCallback();
    void destroyDebugCallback();
    void queryFunctions();
    void releaseFunctions();
    void setRequestedDeviceExtensions(LayerFeatureFlags flags);
    
    LayerFeatureFlags m_enabledLayers;
    LayerFeatureFlags m_requestedDeviceFeatures;
    VkInstance m_instance;
    std::string m_engineName;
    std::string m_appName;
    U32         m_engineVersion;
    U32         m_appVersion;

    // Callback if needed.
    VkDebugReportCallbackEXT m_debugReportCallback;
};
} // Recluse