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
#define QUALCOMM_VENDOR_ID      0x5143

namespace Recluse {


struct R_PUBLIC_API ApplicationInfo 
{
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


class GraphicsInstance 
{
public:
    GraphicsInstance(GraphicsAPI api)
        : m_api(api) { }

    virtual                                     ~GraphicsInstance() { }
    
    // Create the graphics instance. This should be the starting point for your graphics rendering 
    // devices. Only one instance is allowed to be created at a time. If you desire to use a different
    // API during runtime, be sure to destroy the current instance first, before creating a new one.
    static R_PUBLIC_API GraphicsInstance*       create(enum GraphicsAPI api = GraphicsApi_Vulkan);

    // Destroy the graphics instance. Be sure to clean up any memory or objects that were allocated by 
    // adapters.
    static R_PUBLIC_API ResultCode              destroyInstance(GraphicsInstance* pInstance);

    R_PUBLIC_API ResultCode                     initialize(const ApplicationInfo& appInfo, LayerFeatureFlags flags) 
    { 
        ResultCode err = onInitialize(appInfo, flags);
        queryGraphicsAdapters();
        return err;
    }

    // Get available adapters.
    R_PUBLIC_API std::vector<GraphicsAdapter*>& getGraphicsAdapters() { return m_graphicsAdapters; }

    // Get the runtime graphics api.
    GraphicsAPI                                 getApi() const { return m_api; }

protected:
    // Called in initialize.
    virtual void                                queryGraphicsAdapters() = 0;

    // Called in destroy.
    virtual void                                freeGraphicsAdapters() = 0;

    // Called in initialize.
    virtual ResultCode                          onInitialize(const ApplicationInfo& appInfo, LayerFeatureFlags flags) = 0;
    
    // Called in destroy.
    virtual void                                onDestroy() = 0;

    // Available adapters.
    std::vector<GraphicsAdapter*>               m_graphicsAdapters;

private:
    // Destroys the context, should be called by the destroyContext() function.
    void                                        destroy() 
    { 
        freeGraphicsAdapters();
        onDestroy();
    }

    // Graphics api used.
    GraphicsAPI                                 m_api;
};
} // Recluse