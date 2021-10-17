//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {


struct MapRange {
    U64 offsetBytes;
    U64 sizeBytes;
};

// Graphics Resource description.
//
class R_PUBLIC_API GraphicsResource {
public:
    GraphicsResource(GraphicsResourceDescription& desc)
        : m_desc(desc) { }

    virtual ~GraphicsResource() { }

    const GraphicsResourceDescription& getDesc() const { return m_desc; }

    // Map the resource based on range. If NULL range, maps the entire resource.
    virtual ErrType map(void** pMappedMemory, MapRange* pReadRange) { return REC_RESULT_NOT_IMPLEMENTED; }

    // Unmap and invalidate the resource cache, which will flush if needed.
    virtual ErrType unmap(MapRange* pWriteRange) { return REC_RESULT_NOT_IMPLEMENTED; }


    ResourceState getCurrentResourceState() const { return m_currentState; }

private:
    GraphicsResourceDescription m_desc;

protected:
    ResourceState m_currentState;
};
} // Recluse