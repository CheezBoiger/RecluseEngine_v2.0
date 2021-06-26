//
#pragma once

#include "Core/Types.hpp"
#include "GraphicsContext.hpp"

namespace Recluse {

enum GraphicsVendor {
    VENDOR_UNKNOWN,
    VENDOR_INTEL,
    VENDOR_NVIDIA,
    VENDOR_AMD
};

struct R_EXPORT AdapterInfo {
    char            deviceName[256];
    U32             vendorId;
    GraphicsVendor  vendor;
};


struct R_EXPORT AdapterLimits {
    
};

struct R_EXPORT DeviceCreateInfo {
    void*               winHandle;
};

class GraphicsDevice;

class R_EXPORT GraphicsAdapter {
public:
    virtual ~GraphicsAdapter() { }

    virtual ErrType getAdapterInfo(AdapterInfo* out) const { return 0; }
    virtual ErrType getAdapterLimits() const { return 0; }

    // Creates a device from this adapter.
    virtual ErrType createDevice(DeviceCreateInfo* info, GraphicsDevice** ppDevice) { return 0; }

    // Destroys the device associated with this adapter.
    virtual ErrType destroyDevice(GraphicsDevice* pDevice, GraphicsContext* pContext) { return 0; }
};



} // Recluse