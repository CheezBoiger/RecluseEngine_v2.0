//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

#include <vector>

namespace Recluse {

class GraphicsDevice;
class GraphicsAdapter;
class GraphicsContext;

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

    void reconstruct();

    void pushRenderCommand(const RenderCommand& renderCommand);

    void render();
    void present();

    const RendererConfigs& getConfigs() const { return m_rendererConfigs; }

    GraphicsDevice* getDevice() const { return m_pDevice; }

private:

    void determineAdapter(std::vector<GraphicsAdapter*>& adapters);

    GraphicsContext* m_pContext;
    GraphicsAdapter* m_pAdapter;
    GraphicsDevice* m_pDevice;

    // Renderer configs.
    RendererConfigs m_rendererConfigs;
};
} // Engine
} // Recluse