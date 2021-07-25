//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

#include <vector>

#define RECLUSE_ENGINE_NAME_STRING "Recluse Engine"
#define RECLUSE_ENGINE_VERSION_MAJOR 1
#define RECLUSE_ENGINE_VERSION_MINOR 0
#define RECLUSE_ENGINE_VERSION_PATCH 0

namespace Recluse {

class GraphicsDevice;
class GraphicsAdapter;
class GraphicsContext;
class GraphicsQueue;
class GraphicsCommandList;
class GraphicsSwapchain;

// Engine namespace.
namespace Engine {

class Texture2D;
class Mesh;
class Primitive;
class VertexBuffer;
class IndexBuffer;
class RenderCommand;

struct MeshDescription {
};


struct MaterialDescription {

};

struct RendererConfigs {
    U32 renderWidth;
    U32 renderHeight;
    GraphicsAPI api;
};

class R_EXPORT Renderer {
public:
    void initialize(void* pWindowHandle, const RendererConfigs& configs);
    void cleanUp();

    void recreate(const RendererConfigs& newConfigs);

    void pushRenderCommand(const RenderCommand& renderCommand);

    void render();
    void present();

    const RendererConfigs& getConfigs() const { return m_rendererConfigs; }

    GraphicsDevice* getDevice() const { return m_pDevice; }


    Texture2D* createTexture2D();
    VertexBuffer* createVertexBuffer();
    IndexBuffer* createIndexBuffer();

private:

    void determineAdapter(std::vector<GraphicsAdapter*>& adapters);

    void createDevice();
    
    void destroyDevice();

    GraphicsContext* m_pContext;
    GraphicsAdapter* m_pAdapter;
    GraphicsDevice* m_pDevice;
    GraphicsQueue* m_graphicsQueue;
    GraphicsCommandList* m_commandList;
    GraphicsSwapchain* m_pSwapchain;

    // Renderer configs.
    RendererConfigs m_rendererConfigs;
    void*           m_windowHandle;
};
} // Engine
} // Recluse