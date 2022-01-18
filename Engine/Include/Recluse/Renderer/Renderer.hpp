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

struct RenderMessage : public JobMessage 
{
public:
    RenderMessage()
        : JobMessage("Renderer")
    { }

    enum Request 
    {
        PAUSE,
        RESUME,
        SHUTDOWN,
        CHANGE_CONFIG,
        SCENE_UPDATE,
    };

    Application* pApp;
    Request req;
};

enum RenderPassType : U32 
{
    RENDER_PREZ = 0x0001,
    RENDER_SHADOW = 0x0002,
    RENDER_PARTICLE = 0x0004,
    RENDER_GBUFFER = 0x0008,
    RENDER_HAS_MATERIAL = 0x0010,
    RENDER_FORWARD_OPAQUE = 0x0020,
    RENDER_FORWARD_TRANSPARENT = 0x0040,
    RENDER_HAZE = 0x0080
};

typedef U32 RenderPassTypeFlags;


enum QualitySetting
{
    QUALITY_NONE,
    QUALITY_LOW,
    QUALITY_MEDIUM,
    QUALITY_HIGH,
    QUALITY_BEST
};

struct MeshDescription 
{
};


struct MaterialDescription 
{

};

struct RendererConfigs 
{
    U32 renderWidth;
    U32 renderHeight;
    U32 buffering;
    GraphicsAPI api;
    Bool enableMotionBlur;
    Bool enableHeatHaze;
    Bool enableChromaticAberration;
    Bool enableScreenSpaceShadows;
    Bool enableSubsurfaceScattering;
    Bool enableParticleMotion;
    QualitySetting shadowQuality;
};


typedef MapContainer<U32, std::vector<U64>> CommandKeyContainer;


// Top level rendering engine. Implements Render Hardware Interface, and 
// manages all resources and states created in game graphics. This will usually
// implement any render passes and stages of the graphics pipeline.
class R_PUBLIC_API Renderer final : public EngineModule<Renderer> 
{
public:
    Renderer();
    ~Renderer();

    void initialize(void* pWindowHandle, const RendererConfigs& configs);
    void cleanUp();

    void recreate(const RendererConfigs& newConfigs);

    void pushRenderCommand(const RenderCommand& renderCommand, RenderPassTypeFlags renderFlags);

    void render();
    void present();

    const RendererConfigs& getConfigs() const { return m_rendererConfigs; }

    GraphicsDevice* getDevice() const { return m_pDevice; }

    VertexBuffer* createVertexBuffer(U64 perVertexSzBytes, U64 totalVertices);
    IndexBuffer* createIndexBuffer(IndexType indexType, U64 totalIndices);
    Texture2D*  createTexture2D(U32 width, U32 height, U32 mips, U32 layers, ResourceFormat format);

    ErrType destroyTexture2D(Texture2D* pTexture);

    ErrType destroyGPUBuffer(GPUBuffer* pBuffer);

    void update(F32 currentTime, F32 deltaTime);

private:

    virtual ErrType onInitializeModule(Application* pApp) override;
    virtual ErrType onCleanUpModule(Application* pApp) override;

    void determineAdapter(std::vector<GraphicsAdapter*>& adapters);
    void setUpModules();
    void cleanUpModules();
    void createDevice(const RendererConfigs& configs);
    void allocateSceneBuffers(const RendererConfigs& configs);

    void interpolateTime();

    void destroyDevice();
    void freeSceneBuffers();

    void resetCommandKeys();
    void sortCommandKeys();

    // Graphics context and information.
    GraphicsInstance*       m_pInstance;
    GraphicsAdapter*        m_pAdapter;
    GraphicsDevice*         m_pDevice;
    GraphicsCommandList*    m_commandList;
    GraphicsSwapchain*      m_pSwapchain;

    // Renderer configs.
    RendererConfigs         m_rendererConfigs;
    void*                   m_windowHandle;

    // Scene buffer objects.
    SceneBufferDefinitions              m_sceneBuffers;
    std::vector<RenderCommandList*>     m_renderCommands;
    U32                                 m_currentFrameIndex;
    U32                                 m_maxBufferCount;

    struct CommandKey 
    {
        union 
        {
            U64 value;
        };
    };

    struct KeySorter 
    {
        U64 start;
        U64 end;

        void pushKey(U64 value);

        void sortKeys();
    };

    struct 
    {
        F32 currentTick;
        F32 currentTime;
        F32 deltaTime;

        F32 fixedTick;
        F32 startTick;
        F32 endTick;
    } m_renderState;

    // command keys identify the index within the render command, to begin rendering for.
    std::vector<std::unordered_map<U32, std::vector<U64>>>  m_commandKeys;
    RenderCommandList*                                      m_currentRenderCommands;
    CommandKeyContainer                                     m_currentCommandKeys;
};
} // Engine
} // Recluse