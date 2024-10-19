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
} // Engine
} // Recluse