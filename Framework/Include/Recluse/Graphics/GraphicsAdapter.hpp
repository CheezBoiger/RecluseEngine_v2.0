//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {


struct R_EXPORT AdapterInfo {
    char            deviceName[256];
    U32             vendorId;
    char*           vendorName;
};


struct R_EXPORT AdapterLimits {
    
};

struct R_EXPORT DeviceCreateInfo {
    void*                       winHandle;
    SwapchainCreateDescription  swapchainDescription; // Swapchain description, if a window handle is present.
    U32                         buffering; // buffered resources count. 
};

class GraphicsDevice;

class R_EXPORT GraphicsAdapter {
public:
    virtual ~GraphicsAdapter() { }

    virtual ErrType getAdapterInfo(AdapterInfo* out) const { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType getAdapterLimits() const { return REC_RESULT_NOT_IMPLEMENTED; }

    // Creates a device from this adapter.
    virtual ErrType createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    // Destroys the device associated with this adapter.
    virtual ErrType destroyDevice(GraphicsDevice* pDevice) { return REC_RESULT_NOT_IMPLEMENTED; }
};



} // Recluse