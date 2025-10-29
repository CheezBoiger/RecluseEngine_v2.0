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


enum RenderModel
{
    // Object is static, does not move or require consistent updates.
    RenderModel_Static,
    // Model is dynamic, requires consistent updates as it moves in the world.
    RenderModel_Dynamic
};


typedef U32 RenderUpdateFlags;


class RendererComponent : public ECS::Component
{
public:
    R_DECLARE_COMPONENT(RendererComponent);

    virtual ~RendererComponent() { }
    RendererComponent(RenderModel renderModel) 
        : m_renderModel(renderModel) { }

    RenderModel         getRenderModel() const { return m_renderModel; }

    // Sends a request to update the gpu resource, that is responsible for rendering the 
    // model transform of the render object. 
    void                updateModelData();

    Bool                isVisible() const { return m_isVisible; }
    RenderModel         getModelType() const { return m_renderModel; }

    void                setVisible(Bool visible) { m_isVisible = visible; }
    void                setModelType(RenderModel model) { m_renderModel = model; }
    
    GraphicsResource*   m_gfxResourceRef;   // Reference to the graphics resource.
    U32                 m_gfxMeshId;        // Mesh index within an instance.
    U32                 m_gfxMatId;         // Material index within an instance.
    RenderUpdateFlags   m_flags;            // Update flags.
    Bool                m_isVisible : 1;    // Is the mesh visible?
    Bool                m_isSkinned : 1;    // Mesh is skinned, requires animation updates and movements.
    Bool                m_isEnabled : 1;    // If the renderable is enabled for drawing.

private:
    RenderModel m_renderModel;
};


class RendererComponentRegistry : public ECS::ComponentRegistry<RendererComponent>
{
public:
    R_DECLARE_COMPONENT_REGISTRY(RendererComponentRegistry);

    ResultCode                          onAllocateComponent(const RGUID& owner) override;
    ResultCode                          onFreeComponent(const RGUID& owner) override;
    RendererComponent*                  getComponent(const RGUID& entityKey) override;
    std::vector<RendererComponent*>     getAllComponents() override;

private:
    std::map<RGUID, RendererComponent*> m_componentTable;
    
};
} // Recluse 