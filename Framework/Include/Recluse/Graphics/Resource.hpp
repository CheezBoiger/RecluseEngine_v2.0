//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {


struct MapRange 
{
    U64 offsetBytes;
    U64 sizeBytes;
};

// Graphics Resource description.
//
class R_PUBLIC_API GraphicsResource : public virtual IGraphicsObject, public ICastableObject
{
public:
    GraphicsResource(GraphicsResourceDescription& desc)
        : m_desc(desc) { }

    virtual ~GraphicsResource() { }

    const GraphicsResourceDescription& getDesc() const { return m_desc; }

    // Map the resource based on range. If NULL range, maps the entire resource.
    virtual ErrType map(void** pMappedMemory, MapRange* pReadRange) { return RecluseResult_NoImpl; }

    // Unmap and invalidate the resource cache, which will flush if needed.
    virtual ErrType unmap(MapRange* pWriteRange) { return RecluseResult_NoImpl; }

    ResourceState getCurrentResourceState() const { return m_currentState; }

    Bool isInResourceState(ResourceState desiredState) const { return (m_currentState == desiredState); }

protected:
    void setCurrentResourceState(ResourceState state) { m_currentState = state; }

private:
    GraphicsResourceDescription m_desc;
    ResourceState m_currentState;
};
} // Recluse