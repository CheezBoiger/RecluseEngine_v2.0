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
    GraphicsResource*   getResource() const { return m_desc.pResource; }
    Bool                hasResource() const { return m_desc.pResource != nullptr; }

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
    virtual SamplerCreateDesc   getDesc() = 0;

};
} // Recluse