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

struct AdapterInfo {
    std::string     deviceName;
    U32             vendorId;
    GraphicsVendor  vendor;
};

class GraphicsAdapter {
public:

    virtual ErrType getAdapterInfo(AdapterInfo* out) const { return 0; }
    virtual ErrType getAdapterLimits() const { return 0; }
};



} // Recluse