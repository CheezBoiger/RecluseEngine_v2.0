//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Game/Component.hpp"

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

namespace Recluse {


class RendererComponent : public ECS::Component
{
public:
    virtual ~RendererComponent() { }
    
    virtual void update(const RealtimeTick& tick) override;

private:
    
    GraphicsResource*   m_gfxResourceRef;

    U32                 m_gfxMeshId;    // Mesh index within an instance.
    U32                 m_gfxMatId;     // Material index within an instance.
};
} // Recluse 