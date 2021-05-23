// 
#pragma once

#include "Core/Types.hpp"
#include <vector>


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


class GraphicsContext {
public:

    R_EXPORT ErrType initialize(const ApplicationInfo& appInfo, EnableLayerFlags flags) { 
        ErrType err = onInitialize(appInfo, flags);
        queryGraphicsAdapters();
        return err;
    }

    R_EXPORT void destroy() { 
        freeGraphicsAdapters();
        onDestroy();
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
};
} // Recluse