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
    GraphicsResource()
        { }

    virtual                             ~GraphicsResource() { }

    // Map the resource based on range. If NULL range, maps the entire resource.
    virtual ResultCode                  map(void** pMappedMemory, MapRange* pReadRange) { return RecluseResult_NoImpl; }

    // Unmap and invalidate the resource cache, which will flush if needed.
    virtual ResultCode                  unmap(MapRange* pWriteRange) { return RecluseResult_NoImpl; }

    ResourceState                       getCurrentResourceState() const { return m_currentState; }

    Bool                                isInResourceState(ResourceState desiredState) const { return (m_currentState == desiredState); }

    // Get the resource as a specific view.
    virtual ResourceViewId              asView(const ResourceViewDescription& description) { return 0; }

protected:
    void                                setCurrentResourceState(ResourceState state) { m_currentState = state; }

private:
    ResourceState                       m_currentState;
    ResourceId                          m_id;
};
} // Recluse