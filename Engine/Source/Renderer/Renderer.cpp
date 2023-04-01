//

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"

#include "Recluse/Messaging.hpp"

#include "PreZRenderModule.hpp"
#include "AOVRenderModule.hpp"
#include "LightClusterModule.hpp"

#include "Recluse/Renderer/RenderCommand.hpp"

#include "Recluse/Generated/RendererResources.hpp"

#include <algorithm>

#define R_NULLIFY_RENDER 1

namespace Recluse {
namespace Engine {


DEFINE_ENGINE_MODULE(Renderer);

Renderer::Renderer()
{
}


Renderer::~Renderer()
{
}


void Renderer::initialize()
{
    R_ASSERT_FORMAT(m_newRendererConfigs.buffering >= 1, "Must at least be one buffer count!");
    // Immediately initialize the render configs to the current.
    m_currentRendererConfigs = m_newRendererConfigs;

    LayerFeatureFlags flags  = 0;
    ApplicationInfo info    = { };
    ErrType result          = RecluseResult_Ok;
    m_windowHandle          = m_currentRendererConfigs.windowHandle;

    m_pInstance = GraphicsInstance::createInstance(m_currentRendererConfigs.api);
    
    if (!m_pInstance) 
    {
        R_ERR("Renderer", "Failed to create graphics context, aborting...");

        return;
    }

    info.engineName     = RECLUSE_ENGINE_NAME_STRING;
    info.engineMajor    = RECLUSE_ENGINE_VERSION_MAJOR;
    info.engineMinor    = RECLUSE_ENGINE_VERSION_MINOR;
    info.enginePatch    = RECLUSE_ENGINE_VERSION_PATCH;
    info.appName        = "NoName";

    result = m_pInstance->initialize(info, flags);

    if (result != RecluseResult_Ok) 
    {
        R_ERR("Renderer", "Failed to initialize instance!");        
    }

    std::vector<GraphicsAdapter*> adapters = m_pInstance->getGraphicsAdapters();
    
    determineAdapter(adapters);

    createDevice(m_currentRendererConfigs);
    m_pContext = m_pDevice->createContext();
    m_pSwapchain = m_pDevice->getSwapchain();

    {
        MemoryReserveDescription reserveDesc = { };
        reserveDesc.bufferPools[ResourceMemoryUsage_CpuOnly]     = R_1MB * 32ull;
        reserveDesc.bufferPools[ResourceMemoryUsage_CpuToGpu]   = R_1MB * 16ull;
        reserveDesc.bufferPools[ResourceMemoryUsage_GpuToCpu]   = R_1MB * 16ull;
        reserveDesc.bufferPools[ResourceMemoryUsage_GpuOnly]     = R_1MB * 512ull;
        reserveDesc.texturePoolGPUOnly                              = R_1GB * 1ull;

        // Memory reserves for engine.
        result = m_pDevice->reserveMemory(reserveDesc);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR("Renderer", "ReserveMemory call completed with result: %d", result);
    }

    allocateSceneBuffers(m_currentRendererConfigs);

    setUpModules();
}


void Renderer::cleanUp()
{
    freeSceneBuffers();

    // Clean up all modules, as well as resources handled by them...
    cleanUpModules();
    m_pDevice->releaseContext(m_pContext);
    if (m_pDevice) 
    {
        m_pAdapter->destroyDevice(m_pDevice);
    }
}


void Renderer::present(Bool delayPresent)
{
    ErrType result = m_pSwapchain->present(delayPresent ? GraphicsSwapchain::PresentConfig_SkipPresent : GraphicsSwapchain::PresentConfig_Present);

    if (result != RecluseResult_Ok) 
    {
        R_WARN("Renderer", "Swapchain present returns with err code: %d", result);
    }
}


void Renderer::render()
{
    sortCommandKeys();
    GraphicsContext* context = getContext();

    context->begin();
#if (!R_NULLIFY_RENDER)
        // TODO: Would make more sense to manually transition the resource itself, 
        //       and not the resource view...
        GraphicsResource* pSceneDepth = m_sceneBuffers.pSceneDepth->getResource();
        //GraphicsResource* pSceneAlbedo = m_sceneBuffers.pSceneAlbedo->getResource();
        
        if (pSceneDepth->getCurrentResourceState() != RESOURCE_STATE_DEPTH_STENCIL_WRITE) 
        {
            // Transition the resource.
            ResourceTransition trans        = MAKE_RESOURCE_TRANSITION(pSceneDepth, RESOURCE_STATE_DEPTH_STENCIL_WRITE, 0, 1, 0, 1);
            //ResourceTransition albedoTrans  = MAKE_RESOURCE_TRANSITION(pSceneAlbedo, RESOURCE_STATE_RENDER_TARGET, 0, 1, 0, 1);
            m_commandList->transition(&trans, 1);            
        }

        PreZ::generate
                (
                    m_commandList, 
                    m_currentRenderCommands, 
                    m_currentCommandKeys[RENDER_PREZ].data(), 
                    m_currentCommandKeys[RENDER_PREZ].size()
                );

        // Re-transition back to read only.
        if (pSceneDepth->getCurrentResourceState() != RESOURCE_STATE_DEPTH_STENCIL_READONLY) 
        {
            ResourceTransition trans = MAKE_RESOURCE_TRANSITION(pSceneDepth, RESOURCE_STATE_DEPTH_STENCIL_READONLY, 0, 1, 0, 1);
            m_commandList->transition(&trans, 1);
        }

        // Asyncronous Queue -> Do Light culling here.
        LightCluster::cullLights(m_commandList);

        AOV::generate
                (
                    m_commandList, 
                    m_currentRenderCommands,
                    m_currentCommandKeys[RENDER_GBUFFER].data(), 
                    m_currentCommandKeys[RENDER_GBUFFER].size()
                );

        // Deferred rendering combine.
        LightCluster::combineDeferred(m_commandList);

        // Forward pass combine.
        LightCluster::combineForward
                        (
                            m_commandList, 
                            m_currentCommandKeys[RENDER_FORWARD_OPAQUE].data(), 
                            m_currentCommandKeys[RENDER_FORWARD_OPAQUE].size()
                        );
#endif
    context->end();
    // Present.
    m_pSwapchain->present();
    
    resetCommandKeys();
}


void Renderer::determineAdapter(std::vector<GraphicsAdapter*>& adapters)
{
    for (U32 i = 0; i < adapters.size(); ++i) 
    {
        AdapterInfo info            = { };
        AdapterLimits limits        = { };
        GraphicsAdapter* pAdapter   = adapters[i];
        ErrType result              = RecluseResult_Ok;

        result = pAdapter->getAdapterInfo(&info);
    
        if (result != RecluseResult_Ok) 
        {
            R_ERR("Renderer", "Failed to query adapter info.");
        }

        R_DEBUG("Adapter Name: %s\n\t\tVendor: %s", info.deviceName, info.vendorName);

        // Just really quickly, pick up the first adapter.
        m_pAdapter = pAdapter;
        break;
    }
}


void Renderer::createDevice(const RendererConfigs& configs)
{
    DeviceCreateInfo info                   = { };
    info.winHandle                          = m_windowHandle;
    info.swapchainDescription.buffering     = FrameBuffering_Single;
    info.swapchainDescription.desiredFrames = 3;
    info.swapchainDescription.format        = ResourceFormat_R8G8B8A8_Unorm;
    info.swapchainDescription.renderWidth   = configs.renderWidth;
    info.swapchainDescription.renderHeight  = configs.renderHeight;

    ErrType result          = RecluseResult_Ok;
    
    result = m_pAdapter->createDevice(info, &m_pDevice);

    if (result != RecluseResult_Ok) 
    {
        R_ERR("Renderer", "Failed to create device!");   
    }
}


void Renderer::setUpModules()
{
    m_sceneBuffers.gbuffer[Engine::GBuffer_Depth] = new Texture2D();
    m_sceneBuffers.gbuffer[Engine::GBuffer_Depth]->initialize
                                    (
                                        this, 
                                        ResourceFormat_D32_Float_S8_Uint, 
                                        m_currentRendererConfigs.renderWidth, 
                                        m_currentRendererConfigs.renderHeight, 
                                        1, 1
                                    );

    //PreZ::initialize(m_pDevice, &m_sceneBuffers);
}


void Renderer::cleanUpModules()
{
    //PreZ::destroy(m_pDevice);
}


VertexBuffer* Renderer::createVertexBuffer(U64 perVertexSzBytes, U64 totalVertices)
{
    ErrType result          = RecluseResult_Ok;
    VertexBuffer* pBuffer   = new VertexBuffer();

    result = pBuffer->initializeVertices(m_pDevice, perVertexSzBytes, totalVertices);

    if (result != RecluseResult_Ok) 
    {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}


IndexBuffer* Renderer::createIndexBuffer(IndexType indexType, U64 totalIndices)
{
    ErrType result = RecluseResult_Ok;
    IndexBuffer* pBuffer = new IndexBuffer();
    
    result = pBuffer->initializeIndices(m_pDevice, indexType, totalIndices);

    if (result != RecluseResult_Ok) 
    {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}


ErrType Renderer::destroyGPUBuffer(GPUBuffer* pBuffer)
{
    ErrType result = RecluseResult_Ok;

    if (!pBuffer) 
    {
        return RecluseResult_Failed;
    }

    result = pBuffer->destroy();
    delete pBuffer;

    return result;
}


void Renderer::resetCommandKeys()
{
    m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxBufferCount;
    
    m_currentCommandKeys = CommandKeyContainer(&m_commandKeys[m_currentFrameIndex]);
    m_currentRenderCommands = m_renderCommands[m_currentFrameIndex];
    
    R_ASSERT(m_currentCommandKeys.isValid());

    for (auto& cmdLists : m_currentCommandKeys.get()) 
    {
        cmdLists.second.clear();
    }
}


void Renderer::sortCommandKeys()
{
    R_ASSERT(m_currentCommandKeys.isValid());

    struct Cmp 
    { 
        bool operator()(const U64 a, const U64 b) const 
        { 
            return a < b;
        }
    } pred;

    for (auto& cmdLists : m_currentCommandKeys.get()) 
    {
        std::vector<U64>& list = cmdLists.second;
        std::sort(list.begin(), list.end(), pred);
    }
}


void Renderer::pushRenderCommand(const RenderCommand& renderCommand, RenderPassTypeFlags renderFlags)
{
    R_ASSERT(m_currentRenderCommands != NULL);

    // Store mesh commands to be referenced for each draw pass.
    CommandKey key                  = { m_currentRenderCommands->getNumberCommands() };

    if (renderFlags & Render_PreZ) 
    {
        m_currentCommandKeys[Render_PreZ].push_back(key.value);
    }

    if (renderFlags & Render_Gbuffer) 
    {
        m_currentCommandKeys[Render_Gbuffer].push_back(key.value);
    }

    if (renderFlags & Render_Shadow) 
    {
        m_currentCommandKeys[Render_Shadow].push_back(key.value);
    }

    // Mesh is treated as particles.
    if (renderFlags & Render_Particles) 
    {
        m_currentCommandKeys[Render_Particles].push_back(key.value);
    }

    // Push the render command to the last, this will serve as our reference to render in
    // certain render passes.
    m_currentRenderCommands->push(renderCommand);
}


void Renderer::allocateSceneBuffers(const RendererConfigs& configs)
{
    U32 width = configs.renderWidth;
    U32 height = configs.renderHeight;
    m_sceneBuffers.gbuffer[Engine::GBuffer_Depth] = createTexture2D(width, height, 1, 1, ResourceFormat_D24_Unorm_S8_Uint);

    ResourceViewDescription viewDesc = { };
    viewDesc.dimension = ResourceViewDimension_2d;
    viewDesc.format = ResourceFormat_D24_Unorm_S8_Uint;
    viewDesc.layerCount = 1;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.baseMipLevel = 0;
    viewDesc.type = ResourceViewType_DepthStencil;

    m_sceneBuffers.gbufferViews[Engine::GBuffer_Depth] = new TextureView();
    m_sceneBuffers.gbufferViews[Engine::GBuffer_Depth]->initialize(this, m_sceneBuffers.gbuffer[Engine::GBuffer_Depth], viewDesc);

    m_renderCommands.resize(configs.buffering);
    m_maxBufferCount = configs.buffering;
    for (U32 i = 0; i < configs.buffering; ++i)
    {
        m_renderCommands[i] = new RenderCommandList();
        m_renderCommands[i]->initialize();
    }

    m_commandKeys.resize(m_maxBufferCount);

    m_currentRenderCommands = m_renderCommands[0];
    m_currentCommandKeys = CommandKeyContainer(&m_commandKeys[0]);
}


void Renderer::freeSceneBuffers()
{

    if (m_sceneBuffers.gbufferViews[Engine::GBuffer_Depth]) 
    {
        m_sceneBuffers.gbufferViews[Engine::GBuffer_Depth]->destroy(this);
        delete m_sceneBuffers.gbufferViews[Engine::GBuffer_Depth];
        m_sceneBuffers.gbufferViews[Engine::GBuffer_Depth] = nullptr;
    }

    if (m_sceneBuffers.gbuffer[Engine::GBuffer_Depth]) 
    {
    
        destroyTexture2D(m_sceneBuffers.gbuffer[Engine::GBuffer_Depth]);
        m_sceneBuffers.gbuffer[Engine::GBuffer_Depth] = nullptr;

    }

    for (U64 i = 0; i < m_renderCommands.size(); ++i) 
    {
        m_renderCommands[i]->destroy();
        delete m_renderCommands[i];
    }
    m_renderCommands.clear();
}


Texture2D* Renderer::createTexture2D(U32 width, U32 height, U32 mips, U32 layers, ResourceFormat format)
{
    Texture2D* pTexture = new Texture2D();
    ErrType result = RecluseResult_Ok;

    result = pTexture->initialize(this, format, width, height, layers, mips);

    if (result != RecluseResult_Ok) 
    {
        pTexture->destroy(this);
        delete pTexture;
    }
  
    return pTexture;
}


ErrType Renderer::destroyTexture2D(Texture2D* pTexture)
{
    R_ASSERT(pTexture != NULL);

    pTexture->destroy(this);
    delete pTexture;

    return RecluseResult_Ok;
}


static ErrType kRendererJob(void* pData)
{
    Renderer* pRenderer                     = Renderer::getMain();
    Mutex renderMutex                       = pRenderer->getMutex();
    U64 threadId                            = getCurrentThreadId();
    F32 counterFrameMs                      = 0.f;

        // Initialize the renderer watch.
    RealtimeTick::initializeWatch(threadId, JOB_TYPE_RENDERER);

    // Initialize module here.
    while (pRenderer->isActive()) 
    {
        ScopedLock lck(renderMutex);
        RealtimeTick::updateWatch(threadId, JOB_TYPE_RENDERER);

        RealtimeTick tick = RealtimeTick::getTick(JOB_TYPE_RENDERER);

        // Render interpolation is required.
        if (pRenderer->isRunning()) 
        {
            const RendererConfigs& renderConfigs    = pRenderer->getCurrentConfigs();
            pRenderer->update(tick.getCurrentTimeS(), tick.delta());
            pRenderer->render();

            // Check if we need to limit our frame rate.
            GraphicsSwapchain::PresentConfig presentConfig = GraphicsSwapchain::PresentConfig_SkipPresent;

            if (renderConfigs.maxFrameRate > 0)
            {
                const F32 desiredFramesMs = 1.0f / renderConfigs.maxFrameRate;
                if (counterFrameMs >= desiredFramesMs)
                {
                    counterFrameMs = 0.f;
                    presentConfig = GraphicsSwapchain::PresentConfig_Present;
                }
            }
            else
            {
                // Not framerate limit, so we can present as fast as we can.
                presentConfig = GraphicsSwapchain::PresentConfig_Present;
            }

            pRenderer->present(presentConfig);
        }
    }

    return RecluseResult_Ok;
}


ErrType Renderer::onInitializeModule(Application* pApp)
{
    m_configLock = createMutex();

    MainThreadLoop::getMessageBus()->addReceiver
        (
            "Renderer", [=] (EventMessage* pMsg) -> void 
                { 
                    EventId ev = pMsg->getEvent();
                    R_DEBUG("Renderer", "Received message!");
                    if (isActive()) 
                    {
                        // Handle the message.
                        switch (ev) 
                        {
                            case RenderEvent_Resume:
                                enableRunning(true);
                                break;

                            case RenderEvent_Pause:
                                enableRunning(false);
                                break;

                            case RenderEvent_Shutdown: 
                            {
                                enableRunning(false);
                                cleanUpModule(MainThreadLoop::getApp());
                                break;
                            }

                            case RenderEvent_ConfigureRenderer:
                            {
                                recreate();
                                break;
                            }

                            case RenderEvent_SceneUpdate:
                                break;

                            case RenderEvent_Initialize:
                            {
                                initialize();
                            }

                            default:
                                break;
                        }
                    }
               }
        );

    return pApp->loadJobThread(JOB_TYPE_RENDERER, kRendererJob);
}


ErrType Renderer::onCleanUpModule(Application* pApp)
{
    ScopedLock lck(getMutex());
    cleanUp();
    enableRunning(false);
    // be sure to destroy the final config lock.
    destroyMutex(m_configLock);
    return RecluseResult_Ok;
}


void Renderer::destroyDevice()
{
    if (m_pDevice) 
    {
        m_pAdapter->destroyDevice(m_pDevice);
        m_pDevice = nullptr;
    } 
    R_DEBUG_WRAP
        ( else 
            { 
                R_WARN("Renderer", "No such graphics device exists for this renderer."); 
            }
        )
}


void Renderer::update(F32 currentTime, F32 deltaTime)
{
    // Current time is just the time given during app's life.
    m_renderState.currentTime   = currentTime;
    m_renderState.deltaTime     = deltaTime;

    const F32 currentTick       = m_renderState.currentTime;
    const F32 deltaTick         = m_renderState.deltaTime;
    const F32 endTick           = m_renderState.endTick;
    const F32 startStart        = m_renderState.startTick;
    const F32 fixedTick         = m_renderState.fixedTick;

    F32 interpTick              = currentTick + deltaTime;

    if (interpTick >= endTick)
    {
        // New time block, must update.
        m_renderState.startTick = endTick;
        m_renderState.endTick   = endTick + fixedTick;
    }

    m_renderState.currentTick = interpTick;
}


void Renderer::recreate()
{
    ScopedLock lck(m_configLock);

    R_NO_IMPL();
}
} // Engine
} // Recluse