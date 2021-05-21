// Recluse 
#pragma once

#include "Core/Types.hpp"
#include "Graphics/GraphicsContext.hpp"
#include <vulkan/vulkan.h>


namespace Recluse {

class VulkanContext : public GraphicsContext {
public:

    // Initialize the vulkan context.
    ErrType onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags layerFlags) override;

    // Destroy the vulkan context. Be sure to clean up all resources and device, before
    // destroying vulkan context.
    void onDestroy() override;

    // Getter functions.
    VkInstance get() {
        return m_instance;
    }

    VkInstance get() const {
        return m_instance;
    }

    VkInstance operator()() {
        return get();
    }

    VkInstance operator()() const {
        return get();
    }

    // Get process address.
    PFN_vkVoidFunction getProcAddr(const char* funcName);

    std::string getAppName() { return m_appName; }
    std::string getEngineName() { return m_engineName; }

    void queryGraphicsAdapters() override;
    void freeGraphicsAdapters() override;

    
private:

    void nullify();

    VkInstance m_instance;
    std::string m_engineName;
    std::string m_appName;
    U32         m_engineVersion;
    U32         m_appVersion;
};
} // Recluse