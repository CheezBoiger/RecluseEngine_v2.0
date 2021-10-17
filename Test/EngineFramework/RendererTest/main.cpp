
#include <iostream>

#include "Recluse/System/Input.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Renderer/RenderCommand.hpp"

#include <vector>
#include <queue>

using namespace Recluse;
using namespace Recluse::Engine;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();
    
    Window* pWindow = Window::create("Renderer Test", 0, 0, 512, 512);
    Renderer* pRenderer = new Renderer();

    RendererConfigs configs = { };
    configs.api = GRAPHICS_API_VULKAN;
    configs.buffering = 3;
    configs.renderWidth = pWindow->getWidth();
    configs.renderHeight = pWindow->getHeight();

    pRenderer->initialize(pWindow->getNativeHandle(), configs);

    pWindow->open();

    while (!pWindow->shouldClose()) {
        DrawRenderCommand rcmd = { };
        rcmd.op = COMMAND_OP_DRAWABLE_INSTANCED;
        rcmd.flags = RENDER_PREZ;
        rcmd.vertexTypeFlags = VERTEX_ATTRIB_POSITION | VERTEX_ATTRIB_NORMAL;
        rcmd.numSubMeshes = 0;

        pRenderer->pushRenderCommand(rcmd);

        pRenderer->render();

        pRenderer->present();
        pollEvents();
    }

    delete pRenderer;
    Window::destroy(pWindow);

    Log::destroyLoggingSystem();
    return 0;
}