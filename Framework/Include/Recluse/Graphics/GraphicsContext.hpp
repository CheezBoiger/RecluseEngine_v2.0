// 
#pragma once

#include "Recluse/Types.hpp"
#include <vector>

// Vendor IDs
#define NVIDIA_VENDOR_ID        0x10DE
#define INTEL_VENDOR_ID         0x1F96
#define AMD_VENDOR_ID           0x1022

namespace Recluse {


enum R_EXPORT LayerFeatures {
    LAYER_FEATURE_RAY_TRACING_BIT   = (1 << 0),
    LAYER_FEATURE_MESH_SHADING_BIT  = (1 << 1),
    LAYER_FEATURE_DEBUG_VALIDATION_BIT         = (1 << 2),
    LAYER_FEATURE_API_DUMP_BIT      = (1 << 3)
};


typedef U32 EnableLayerFlags;


struct R_EXPORT ApplicationInfo {
    const char* appName;
    const char* engineName;
    U32         appMajor : 10;
    U32         appMinor : 10;
    U32         appPatch : 12;
    U32         engineMajor : 10;
    U32         engineMinor : 10;
    U32         enginePatch : 12;
};


class GraphicsAdapter;

enum GraphicsAPI {
    GRAPHICS_API_SOFTWARE,
    GRAPHICS_API_VULKAN,
    GRAPHICS_API_OPENGL,
    GRAPHICS_API_D3D11,
    GRAPHICS_API_D3D12
};

class GraphicsContext {
public:

    static R_EXPORT GraphicsContext* createContext(enum GraphicsAPI api = GRAPHICS_API_VULKAN);
    static R_EXPORT ErrType destroyContext(GraphicsContext* pContext);

    R_EXPORT ErrType initialize(const ApplicationInfo& appInfo, EnableLayerFlags flags) { 
        ErrType err = onInitialize(appInfo, flags);
        queryGraphicsAdapters();
        return err;
    }

    // Get available adapters.
    R_EXPORT std::vector<GraphicsAdapter*>& getGraphicsAdapters() { return m_graphicsAdapters; }

protected:
    // Called in initialize.
    virtual void queryGraphicsAdapters() = 0;

    // Called in destroy.
    virtual void freeGraphicsAdapters() = 0;

    // Called in initialize.
    virtual ErrType onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags) = 0;
    
    // Called in destroy.
    virtual void onDestroy() = 0;

    // Available adapters.
    std::vector<GraphicsAdapter*> m_graphicsAdapters;

private:
    // Destroys the context, should be called by the destroyContext() function.
    void destroy() { 
        freeGraphicsAdapters();
        onDestroy();
    }
};
} // Recluse