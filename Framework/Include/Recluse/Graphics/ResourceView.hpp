//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/Format.hpp"

namespace Recluse {

class GraphicsResource;

class GraphicsResourceView 
{
public:
    virtual ~GraphicsResourceView() { }

    GraphicsResourceView(const ResourceViewDesc& desc) 
        : m_desc(desc) { }

    ResourceViewDesc getDesc() const { return m_desc; }

    GraphicsResource* getResource() const { return m_desc.pResource; }

    Bool hasResource() const { return m_desc.pResource != nullptr; }
    
private:
    ResourceViewDesc m_desc;
};


class GraphicsSampler 
{
public:
    virtual ~GraphicsSampler() { }

    // Get a copy.
    virtual SamplerCreateDesc getDesc() = 0;
};
} // Recluse