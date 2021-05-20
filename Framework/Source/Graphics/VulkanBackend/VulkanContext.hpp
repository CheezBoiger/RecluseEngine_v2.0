// Recluse 
#pragma once

#include "Core/Types.hpp"
#include <vulkan/vulkan.h>


namespace Recluse {

enum LayerFeatures {
    LAYER_FEATURE_RAY_TRACING_BIT   = (1 << 0),
    LAYER_FEATURE_MESH_SHADING_BIT  = (1 << 1),
    LAYER_FEATURE_DEBUG_VALIDATION_BIT         = (1 << 2),
    LAYER_FEATURE_API_DUMP_BIT      = (1 << 3)
};

typedef U32 EnableLayerFlags;

struct ApplicationInfo {
    const char* appName;
    const char* engineName;
    U32         appMajor : 10;
    U32         appMinor : 10;
    U32         appPatch : 12;
    U32         engineMajor : 10;
    U32         engineMinor : 10;
    U32         enginePatch : 12;
};

class VulkanContext {
public:

    ErrType initialize(const ApplicationInfo& appInfo, EnableLayerFlags layerFlags);

    void destroy();

    VkInstance get() {
        return m_instance;
    }

    VkInstance operator()() {
        return get();
    }

    PFN_vkVoidFunction getProcAddr(const char* funcName);

private:

    void nullify();

    VkInstance m_instance;
    std::string m_engineName;
    std::string m_appName;
    U32         m_engineVersion;
    U32         m_appVersion;
};
} // Recluse