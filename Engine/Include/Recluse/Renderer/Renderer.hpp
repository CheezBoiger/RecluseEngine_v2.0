//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/EngineModule.hpp"
#include "Recluse/Renderer/RendererResources.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Application.hpp"

#include "Recluse/Structures/HashMap.hpp"

#include "Recluse/Generated/RendererConfigs.hpp"
#include "Recluse/Generated/RendererPrograms.hpp"

#include <vector>
#include <functional>
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
class DebugRenderer;


enum RenderPassType : U32 
{
    Render_PreZ = 0x0001,
    Render_Shadow = 0x0002,
    Render_Particles = 0x0004,
    Render_Gbuffer = 0x0008,
    Render_Material = 0x0010,
    Render_ForwardOpaque = 0x0020,
    Render_ForwardTransparent = 0x0040,
    Render_Haze = 0x0080
};

typedef U32 RenderPassTypeFlags;

struct MeshDescription 
{
};


struct MaterialDescription 
{

};


typedef MapContainer<U32, std::vector<U64>> CommandKeyContainer;


// Top level rendering engine. Implements Render Hardware Interface, and 
// manages all resources and states created in game graphics. This will usually
// implement any render passes and stages of the graphics pipeline.
class R_PUBLIC_API Renderer final : public EngineModule<Renderer> 
{
public:

    typedef std::function<ResultCode(DebugRenderer*)> DebugDrawFunction;
    typedef std::function<ResultCode(Renderer*)> DebugInitFunction;

    Renderer();
    ~Renderer();

    // Initializes the debug plugin for DebugRenderer use.
    ResultCode                  initializeDebugPlugin(DebugInitFunction pluginFunc);

    void                        initialize();
    void                        cleanUp();

    // Recreate the renderer pipeline, with the new configurations.
    void                        recreate();

    // Push the render command to the rendering engine. This will store the command for the drawing frame.
    void                        pushRenderCommand(const RenderCommand& renderCommand, RenderPassTypeFlags renderFlags);
    void                        pushDebugDraw(DebugDrawFunction debugDrawFunction);

    void                        render();
    void                        present(Bool delayPresent = false);

    const RendererConfigs&      getCurrentConfigs() const { return m_currentRendererConfigs; }
    GraphicsContext*            getContext() { return m_pContext; }

    // Set the new configurations for the renderer. This won't be used until we call recreate().
    // Call will be blocked if we are in the middle of recreating.
    void                        setNewConfigurations(const RendererConfigs& newConfigs) 
    { 
        ScopedLock lck(m_configLock); 
        m_newRendererConfigs = newConfigs; 
    }

    GraphicsDevice*             getDevice() const { return m_pDevice; }

    VertexBuffer*               createVertexBuffer(U64 perVertexSzBytes, U64 totalVertices);
    IndexBuffer*                createIndexBuffer(IndexType indexType, U64 totalIndices);
    Texture2D*                  createTexture2D(U32 width, U32 height, U32 mips, U32 layers, ResourceFormat format);

    // Creation of temporary resource, to be used only on the current frame. Will be destroyed on re-draw.
    ResultCode                  createTempResource(const GraphicsResourceDescription& desc);

    ResultCode                  destroyTexture2D(Texture2D* pTexture);
    ResultCode                  destroyGPUBuffer(GPUBuffer* pBuffer);

    void                        update(F32 currentTime, F32 deltaTime);

private:

    virtual ResultCode          onInitializeModule(Application* pApp) override;
    virtual ResultCode          onCleanUpModule(Application* pApp) override;

    void                        determineAdapter(std::vector<GraphicsAdapter*>& adapters);
    void                        setUpModules();
    void                        cleanUpModules();
    void                        createDevice(const RendererConfigs& configs);
    void                        allocateSceneBuffers(const RendererConfigs& configs);
    
    void                        clear();

    void                        interpolateTime();

    void                        destroyDevice();
    void                        freeSceneBuffers();

    void                        resetCommandKeys();
    void                        sortCommandKeys();

    // Graphics context and information.
    GraphicsInstance*                   m_pInstance;
    GraphicsAdapter*                    m_pAdapter;
    GraphicsDevice*                     m_pDevice;
    GraphicsSwapchain*                  m_pSwapchain;
    GraphicsContext*                    m_pContext;

    // Renderer configs.
    Mutex                               m_configLock;
    RendererConfigs                     m_currentRendererConfigs;
    RendererConfigs                     m_newRendererConfigs;
    void*                               m_windowHandle;

    // Scene buffer objects.
    SceneBufferDefinitions              m_sceneBuffers;
    std::vector<RenderCommandList*>     m_renderCommands;
    U32                                 m_currentFrameIndex;
    U32                                 m_maxBufferCount;

    struct CommandKey 
    {
        union 
        {
            U64     value;
        };
    };

    struct KeySorter 
    {
        U64         start;
        U64         end;
        void                            pushKey(U64 value);
        void                            sortKeys();
    };

    struct 
    {
        F32     currentTick;
        F32     currentTime;
        F32     deltaTime;

        F32     fixedTick;
        F32     startTick;
        F32     endTick;
    }                                   m_renderState;

    // command keys identify the index within the render command, to begin rendering for.
    std::vector<std::unordered_map<U32, std::vector<U64>>>  m_commandKeys;
    RenderCommandList*                                      m_currentRenderCommands;
    CommandKeyContainer                                     m_currentCommandKeys;
    std::vector<DebugDrawFunction>                          m_debugDrawFunctions;

    // Gpu Resources used as temporary for the current frame. This will be refreshed every new frame.
    std::vector<GraphicsResource*>                          m_tempResourcesPerFrame;
    std::vector<Allocator*>                                 m_tempResourceAllocatorPerFrame;
};


class ImGuiRenderer : public ModulePlugin<Renderer>
{
public:
    DEFINE_MODULE_PLUGIN(ImGuiRenderer, Renderer, RendererPluginID_DebugRenderer);
    ImGuiRenderer() { }
};


ResourceViewId asView(GraphicsResource* pResource, const ResourceViewDescription& description); 
ResourceViewId asView(GraphicsResource* pResource, ResourceViewType type, ResourceViewDimension dim);
} // Engine
} // Recluse