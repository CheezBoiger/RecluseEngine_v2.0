//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {


struct R_PUBLIC_API AdapterInfo 
{
    char            deviceName[256];
    U32             vendorId;
    char*           vendorName;
};


struct R_PUBLIC_API AdapterLimits 
{
    U32     maxConstBufferBinds;
    U32     maxUavBinds;
    U32     maxSrvBinds;

    Bool    hasGeometryShaders;
    Bool    hasRaytracing;
    Bool    hasMeshShaders;
    Bool    hasAsyncCompute;
};

struct R_PUBLIC_API DeviceCreateInfo 
{
    void*                       winHandle;
    SwapchainCreateDescription  swapchainDescription; // Swapchain description, if a window handle is present.
};

class GraphicsDevice;

class R_PUBLIC_API GraphicsAdapter 
{
public:
    virtual ~GraphicsAdapter() { }

    virtual ResultCode getAdapterInfo(AdapterInfo* out) const { return RecluseResult_NoImpl; }
    virtual ResultCode getAdapterLimits() const { return RecluseResult_NoImpl; }

    // Creates a device from this adapter.
    virtual ResultCode createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) 
        { return RecluseResult_NoImpl; }

    // Destroys the device associated with this adapter.
    virtual ResultCode destroyDevice(GraphicsDevice* pDevice) { return RecluseResult_NoImpl; }
};



} // Recluse