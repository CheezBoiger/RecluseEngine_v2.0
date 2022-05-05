//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Game/Component.hpp"

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

namespace Recluse {


enum RenderUpdateFlags 
{
    RENDER_UPDATE_FLAG_CLEAN                = (1 << 0),
    RENDER_UPDATE_FLAG_VERTEX_BUFFER_UPDATE = (1 << 1),
    RENDER_UPDATE_FLAG_INDEX_BUFFER_UPDATE  = (1 << 2),
    RENDER_UPDATE_FLAG_CONST_BUFFER_UPDATE  = (1 << 3),
    RENDER_UPDATE_FLAG_COLLECT              = (1 << 4)
};


class RendererComponent : public ECS::Component
{
public:
    virtual ~RendererComponent() { }
    
    virtual void onUpdate(const RealtimeTick& tick) override;

private:
    
    GraphicsResource*   m_gfxResourceRef;   // Reference to the graphics.

    U32                 m_gfxMeshId;    // Mesh index within an instance.
    U32                 m_gfxMatId;     // Material index within an instance.
};
} // Recluse 