//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Renderer/RendererResources.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include <vector>
#include <unordered_map>

#define RECLUSE_ENGINE_NAME_STRING "Recluse Engine"
#define RECLUSE_ENGINE_VERSION_MAJOR 1
#define RECLUSE_ENGINE_VERSION_MINOR 0
#define RECLUSE_ENGINE_VERSION_PATCH 0

namespace Recluse {

class GraphicsDevice;
class GraphicsAdapter;
class GraphicsInstance;
class GraphicsQueue;
class GraphicsCommandList;
class GraphicsSwapchain;

// Engine namespace.
namespace Engine {

class Texture2D;
class Mesh;
class Primitive;
struct RenderCommand;
class RenderCommandList;

struct MeshDescription {
};


struct MaterialDescription {

};

struct RendererConfigs {
    U32 renderWidth;
    U32 renderHeight;
    U32 buffering;
    GraphicsAPI api;
};


// Top level rendering engine. Implements Render Harware Interface, and 
// manages all resources and states created in game graphics. This will usually
// implement any render passes and stages of the graphics pipeline.
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
    GraphicsQueue* getGraphicsQueue() const { return m_graphicsQueue; }

    VertexBuffer* createVertexBuffer(U64 perVertexSzBytes, U64 totalVertices);
    IndexBuffer* createIndexBuffer(IndexType indexType, U64 totalIndices);
    Texture2D*  createTexture2D(U32 width, U32 height, U32 mips, U32 layers, ResourceFormat format);

    ErrType destroyTexture2D(Texture2D* pTexture);

    ErrType destroyGPUBuffer(GPUBuffer* pBuffer);

private:

    void determineAdapter(std::vector<GraphicsAdapter*>& adapters);
    void setUpModules();
    void cleanUpModules();
    void createDevice(const RendererConfigs& configs);
    void createSwapchain(const RendererConfigs& configs);
    void allocateSceneBuffers(const RendererConfigs& configs);    

    void destroyDevice();
    void destroySwapchain();
    void freeSceneBuffers();

    void resetCommandKeys();
    void sortCommandKeys();

    // Graphics context and information.
    GraphicsInstance*        m_pInstance;
    GraphicsAdapter*        m_pAdapter;
    GraphicsDevice*         m_pDevice;
    GraphicsQueue*          m_graphicsQueue;
    GraphicsCommandList*    m_commandList;
    GraphicsSwapchain*      m_pSwapchain;

    // Renderer configs.
    RendererConfigs         m_rendererConfigs;
    void*                   m_windowHandle;

    // Scene buffer objects.
    SceneBuffers            m_sceneBuffers;
    RenderCommandList*       m_renderCommands;

    struct CommandKey {
        union {
            U64 value;
        };
    };

    struct KeySorter {
        U64 start;
        U64 end;

        void pushKey(U64 value);

        void sortKeys();
    };

    // command keys identify the index within the render command, to begin rendering for.
    std::unordered_map<U32, std::vector<U64>> m_commandKeys;
};
} // Engine
} // Recluse