//
#pragma once
#include "Recluse/Renderer/Debug/DebugRenderer.hpp"
#include "Recluse/Renderer/Renderer.hpp"

#include "imgui/imgui.h"

namespace Recluse {
namespace Engine {



ResultCode DebugRenderer::createTempResource(const GraphicsResourceDescription& description)
{
    return renderer->createTempResource(description);
}


GraphicsContext* DebugRenderer::getContext()
{
    return renderer->getContext();
}


void DebugRenderer::drawText(U32 x, U32 y, F32 scale, const char* text, const Math::Color4& color)
{
    DebugDrawCallbacks* callbacks = renderer->getDebugCallbacks();
    callbacks->drawTextFn(getContext(), x, y, scale, text, color);
}
} // Engine
} // Recluse