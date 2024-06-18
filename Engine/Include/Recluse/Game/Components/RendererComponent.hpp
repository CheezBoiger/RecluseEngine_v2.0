//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Game/Component.hpp"

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

namespace Recluse {


enum RenderUpdateFlag 
{
    RenderUpdateFlag_Clean          = (1 << 0),
    RenderUpdateFlag_VertexBuffer   = (1 << 1),
    RenderUpdateFlag_IndexBuffer    = (1 << 2),
    RenderUpdateFlag_ConstantBuffer = (1 << 3),
    RenderUpdateFlag_Collect        = (1 << 4)
};


typedef U32 RenderUpdateFlags;


class RendererComponent : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(RendererComponent);

    virtual ~RendererComponent() { }
    RendererComponent() { }
    
    GraphicsResource*   m_gfxResourceRef;   // Reference to the graphics resource.
    U32                 m_gfxMeshId;    // Mesh index within an instance.
    U32                 m_gfxMatId;     // Material index within an instance.
    RenderUpdateFlags   m_flags;        // Update flags.
    Bool                m_isVisible;    // Is the mesh visible?
};


class RendererComponentRegistry : public ECS::ComponentRegistry<RendererComponent>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(RendererComponentRegistry);

    ResultCode          onAllocateComponent(const RGUID& owner) override;
    ResultCode          onFreeComponent(const RGUID& owner) override;
    RendererComponent*  getComponent(const RGUID& entityKey) override;
    std::vector<RendererComponent*> getAllComponents() override;

private:
    std::map<RGUID, RendererComponent*> m_componentTable;
    
};
} // Recluse 