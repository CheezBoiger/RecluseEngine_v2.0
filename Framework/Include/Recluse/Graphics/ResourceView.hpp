//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/Format.hpp"

namespace Recluse {

class GraphicsResource;

typedef GraphicsId ResourceViewId;

class GraphicsResourceView : public ICastableObject, public IGraphicsObject
{
public:
    virtual ~GraphicsResourceView() { }

    GraphicsResourceView(const ResourceViewDescription& desc) 
        : m_desc(desc)
    { }

    const ResourceViewDescription&    getDesc() const { return m_desc; }

private:
    ResourceViewDescription m_desc;
};


typedef GraphicsId SamplerId;

class GraphicsSampler : public ICastableObject, public IGraphicsObject
{
public:
    GraphicsSampler()
    { }
    
    virtual ~GraphicsSampler() { }

    // Get a copy.
    virtual SamplerDescription   getDesc() = 0;

};
} // Recluse