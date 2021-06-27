//
#pragma once

#include "Core/Types.hpp"
#include "GraphicsDevice.hpp"

namespace Recluse {


// Graphics Resource description.
//
class R_EXPORT GraphicsResource {
public:
    GraphicsResource(GraphicsResourceDescription& desc)
        : m_desc(desc) { }

    virtual ~GraphicsResource() { }

    const GraphicsResourceDescription& getDesc() const { return m_desc; }

private:
    GraphicsResourceDescription m_desc;
};
} // Recluse