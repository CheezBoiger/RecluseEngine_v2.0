//
#pragma once
#include "Recluse/Renderer/Debug/DebugRenderer.hpp"
#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Engine {



TemporaryBuffer DebugRenderer::createTempBuffer(const TemporaryBufferDescription& description)
{
    return renderer->createTemporaryBuffer(description);
}


GraphicsContext* DebugRenderer::getContext()
{
    return renderer->getContext();
}


DebugRenderer* DebugRenderer::get()
{
    RendererModule* renderer = RendererModule::getMain();
    R_ASSERT_FORMAT(renderer != nullptr, "Main renderer does not exist, unable to obtain debug renderer!");

    ModulePlugin<RendererModule>* plugin = RendererModule::getMain()->getPlugin(RendererPluginID_DebugRenderer);
    return dynamic_cast<DebugRenderer*>(plugin);
}


void DebugRenderer::drawText(U32 x, U32 y, F32 scale, const char* text, const Math::Color4& color)
{
    m_drawCalls.push_back(std::move(DrawCall{ LitType_Flat, DrawType_Text }));
}


void DebugRenderer::drawText(const Math::Float3& position, const char* text, F32 scale, const Math::Color4& color)
{
    m_drawCalls.push_back(std::move(DrawCall{ LitType_Flat, DrawType_Text}));
}


void DebugRenderer::drawBox3d(const Math::Bounds3d& bounds, const Math::Matrix44& transform, const Math::Color4& color, LitType lit)
{
}


void DebugRenderer::drawPoint(const Math::Float3& position, F32 scale, const Math::Color4& color)
{
}


void DebugRenderer::drawSphere(const Math::Float3& position, F32 radius, const Math::Color4& color, LitType lit)
{
}


void DebugRenderer::drawLine(const Math::Float3& start, const Math::Float3& end, F32 scale, const Math::Color4& color)
{
}


void DebugRenderer::drawTriangle(const Math::Float3& v0, const Math::Float3& v1, const Math::Float3& v2, bool clockwise)
{
}


void DebugRenderer::drawRay(const Math::Float3& origin, const Math::Float3& direction, F32 scale, const Math::Color4& color)
{
}


void DebugRenderer::drawMesh(PrimitiveTopology topology, 
        const Math::Mat44& worldTransform, 
        const std::vector<Math::Float3>& vertices, 
        const std::vector<U16>& indices, 
        const Math::Color4& color, 
        LitType litType)
{
}
} // Engine
} // Recluse