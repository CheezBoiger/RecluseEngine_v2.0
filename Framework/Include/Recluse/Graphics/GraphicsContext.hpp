// 
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include <vector>

// Vendor IDs
#define NVIDIA_VENDOR_ID        0x10DE
#define INTEL_VENDOR_ID         0x8086
#define AMD_VENDOR_ID           0x1022
#define MSFT_VENDOR_ID          0x1414

namespace Recluse {


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