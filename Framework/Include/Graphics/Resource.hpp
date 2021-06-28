//
#pragma once

#include "Core/Types.hpp"
#include "GraphicsDevice.hpp"
#include "Core/Memory/MemoryPool.hpp"

namespace Recluse {


struct MapRange {
    U64 offsetBytes;
    U64 sizeBytes;
};

typedef 

// Graphics Resource description.
//
class R_EXPORT GraphicsResource {
public:
    GraphicsResource(GraphicsResourceDescription& desc)
        : m_desc(desc) { }

    virtual ~GraphicsResource() { }

    const GraphicsResourceDescription& getDesc() const { return m_desc; }

    // Map the resource based on range. If NULL range, maps the entire resource.
    virtual ErrType map(void** pMappedMemory, MapRange* pReadRange) { return REC_RESULT_NOT_IMPLEMENTED; }

    // Unmap and invalidate the resource cache, which will flush if needed.
    virtual ErrType unmap(MapRange* pWriteRange) { return REC_RESULT_NOT_IMPLEMENTED; }

private:
    GraphicsResourceDescription m_desc;
};
} // Recluse