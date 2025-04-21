//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {


struct AdapterInfo 
{
    char            deviceName[256];
    U32             vendorId;
    char*           vendorName;
};


struct AdapterLimits 
{
    U32     maxConstBufferBinds;
    U32     maxUavBinds;
    U32     maxSrvBinds;
    U32     constantBufferOffsetAlignmentBytes;

    Bool    hasGeometryShaders;
    Bool    hasRaytracing;
    Bool    hasMeshShaders;
    Bool    hasAsyncCompute;
};

struct DeviceCreateInfo 
{
    // Allow asyncronous compute queue if it is available. 
    // If not available, the graphics context will notify if so.
    B32 allowAsyncCompute   : 1;
    B32 reserved0           : 31; //< Reserved for future use.
};

class GraphicsDevice;

class GraphicsAdapter 
{
public:
    virtual ~GraphicsAdapter() { }

    virtual ResultCode getAdapterInfo(AdapterInfo* out) const { return RecluseResult_NoImpl; }
    virtual ResultCode getAdapterLimits() const { return RecluseResult_NoImpl; }

    // Get the offset constant buffer offset alignment in bytes.
    // This is needed if you plan on using one resource and offsetting from it!
    virtual U32 constantBufferOffsetAlignmentBytes() const { return 0ull; }

    // Creates a device from this adapter.
    virtual ResultCode createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) 
        { return RecluseResult_NoImpl; }

    // Destroys the device associated with this adapter.
    virtual ResultCode destroyDevice(GraphicsDevice* pDevice) { return RecluseResult_NoImpl; }
};



} // Recluse