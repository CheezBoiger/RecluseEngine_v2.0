//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {


struct MapRange 
{
    U64 offsetBytes;
    U64 sizeBytes;
};

// Graphics Resource object. This may define a buffer, or image that is ideally handle by the gpu driver.
//
class GraphicsResource : public virtual IGraphicsObject, public ICastableObject
{
public:
    GraphicsResource()
        { }

    virtual                             ~GraphicsResource() { }

    // Map the resource based on range. If NULL range, maps the entire resource.
    // This call should only be made with host visible resources, meaning it must be accessible to the host!
    virtual ResultCode                  map(void** pMappedMemory, MapRange* pReadRange) { return RecluseResult_NoImpl; }

    // Unmap and invalidate the resource cache, which will flush if needed.
    // This call should only be made with host visible resources, meaning it must be accessible to the host!
    virtual ResultCode                  unmap(MapRange* pWriteRange) { return RecluseResult_NoImpl; }

    // Grab the current resource state of this resource.
    ResourceState                       getCurrentResourceState() const { return m_currentState; }

    // Check if this resource is in a given resource state.
    Bool                                isInResourceState(ResourceState desiredState) const { return (m_currentState == desiredState); }

    // Get the resource as a specific view.
    virtual ResourceView                asView(const ResourceViewDescription& description) { return {}; }

    // Get the resource as a constant buffer view.
    virtual ResourceView                asCbv(U32 offsetBytes, U32 sizeBytes) { return {}; }

protected:
    // Probably not recommended to allow public use. This should strictly be used by the given platform.
    void                                setCurrentResourceState(ResourceState state) { m_currentState = state; }

private:
    ResourceState                       m_currentState;
    ResourceId                          m_id;
};
} // Recluse