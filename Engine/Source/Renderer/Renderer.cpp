//

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Limiter.hpp"
#include "Recluse/Renderer/Debug/DebugRenderer.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"

#include "Recluse/Messaging.hpp"

#include "PreZRenderModule.hpp"
#include "AOVRenderModule.hpp"
#include "LightClusterModule.hpp"

#include "Recluse/Renderer/RenderCommand.hpp"

#include "Recluse/Generated/RendererResources.hpp"

#include <algorithm>

#define R_NULLIFY_RENDER 0

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

    LayerFeatureFlags flags  = m_currentRendererConfigs.enableGpuValidation ? (LayerFeatureFlag_GpuDebugValidation | LayerFeatureFlag_DebugValidation) : 0;
    ApplicationInfo info    = { };
    SwapchainCreateDescription swapchainDescription = { };
    ResultCode result          = RecluseResult_Ok;
    m_windowHandle          = m_currentRendererConfigs.windowHandle;

    m_pInstance = GraphicsInstance::create(m_currentRendererConfigs.api);
    
    if (!m_pInstance) 
    {
        R_ERROR("Renderer", "Failed to create graphics context, aborting...");

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
        R_ERROR("Renderer", "Failed to initialize instance!");        
    }

    swapchainDescription.buffering = m_currentRendererConfigs.enableVerticalSyncronization ? FrameBuffering_Double : FrameBuffering_Triple;
    swapchainDescription.desiredFrames = 8;
    swapchainDescription.renderWidth = m_currentRendererConfigs.renderWidth;
    swapchainDescription.renderHeight = m_currentRendererConfigs.renderHeight;
    swapchainDescription.format = ResourceFormat_R8G8B8A8_Unorm;

    std::vector<GraphicsAdapter*> adapters = m_pInstance->getGraphicsAdapters();
    
    determineAdapter(adapters);

    createDevice(m_currentRendererConfigs);
    m_pContext = m_pDevice->createContext();
    m_pContext->setFrames(m_currentRendererConfigs.buffering);
    m_pSwapchain = m_pDevice->createSwapchain(swapchainDescription, m_currentRendererConfigs.windowHandle);

    {
        MemoryReserveDescription reserveDesc = { };
        reserveDesc.bufferPools[ResourceMemoryUsage_CpuVisible] = R_1MB * 32ull;
        reserveDesc.bufferPools[ResourceMemoryUsage_CpuToGpu]   = R_1MB * 16ull;
        reserveDesc.bufferPools[ResourceMemoryUsage_GpuToCpu]   = R_1MB * 16ull;
        reserveDesc.bufferPools[ResourceMemoryUsage_GpuOnly]    = R_1MB * 512ull;
        reserveDesc.texturePoolGPUOnly                          = R_1GB * 1ull;

        // Memory reserves for engine.
        result = m_pDevice->reserveMemory(reserveDesc);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("Renderer", "ReserveMemory call completed with result: %d", result);
    }

    allocateSceneBuffers(m_currentRendererConfigs);

    setUpModules();
}


void Renderer::cleanUp()
{
    m_pContext->wait();
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
    // Present.
    ResultCode result = m_pSwapchain->present(m_pContext);
    if (result == RecluseResult_NeedsUpdate)
    {
        m_pContext->wait();
        m_pSwapchain->rebuild(m_pSwapchain->getDesc());
    }
}


void Renderer::render()
{
    sortCommandKeys();
    GraphicsContext* context = getContext();

    m_pSwapchain->prepare(context);
#if (!R_NULLIFY_RENDER)
        // TODO: Would make more sense to manually transition the resource itself, 
        //       and not the resource view...
        GraphicsResource* pSceneDepth = m_sceneBuffers.gbuffer[GBuffer_Depth]->getResource();
        //GraphicsResource* pSceneAlbedo = m_sceneBuffers.pSceneAlbedo->getResource();
        
        context->transition(pSceneDepth, ResourceState_DepthStencilWrite);

        PreZ::generate
                (
                    context, 
                    m_currentRenderCommands, 
                    m_currentCommandKeys[Render_PreZ].data(), 
                    m_currentCommandKeys[Render_PreZ].size()
                );

        context->transition(pSceneDepth, ResourceState_DepthStencilReadOnly);

        // Asyncronous Queue -> Do Light culling here.
        LightCluster::cullLights(context);

        AOV::generate
                (
                    context, 
                    m_currentRenderCommands,
                    m_currentCommandKeys[Render_Gbuffer].data(), 
                    m_currentCommandKeys[Render_Gbuffer].size()
                );

        // Deferred rendering combine.
        LightCluster::combineDeferred(context);

        // Forward pass combine.
        LightCluster::combineForward
                        (
                            context, 
                            m_currentCommandKeys[Render_ForwardOpaque].data(), 
                            m_currentCommandKeys[Render_ForwardOpaque].size()
                        );
#endif
    // Check if any debug draw functions exist.
    if (!m_debugDrawFunctions.empty())
    {
        // By this state, the debug pass should render on top of the final render target.
        ModulePlugin<Renderer>* plugin = getPlugin(RendererPluginID_DebugRenderer);
        if (plugin)
        {
            DebugRenderer* debugRenderer = dynamic_cast<DebugRenderer*>(plugin);
            for (auto func : m_debugDrawFunctions)
            {
                func(debugRenderer);
            }
        }
    }
    context->transition(m_pSwapchain->getFrame(m_pSwapchain->getCurrentFrameIndex()), ResourceState_Present);
    context->end();
    resetCommandKeys();
    clear();
}


void Renderer::pushDebugDraw(DebugDrawFunction f)
{
    m_debugDrawFunctions.push_back(f);
}


void Renderer::determineAdapter(std::vector<GraphicsAdapter*>& adapters)
{
    for (U32 i = 0; i < adapters.size(); ++i) 
    {
        AdapterInfo info            = { };
        AdapterLimits limits        = { };
        GraphicsAdapter* pAdapter   = adapters[i];
        ResultCode result              = RecluseResult_Ok;

        result = pAdapter->getAdapterInfo(&info);
    
        if (result != RecluseResult_Ok) 
        {
            R_ERROR("Renderer", "Failed to query adapter info.");
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
    //info.winHandle                          = m_windowHandle;
    //info.swapchainDescription.buffering     = FrameBuffering_Single;
    //info.swapchainDescription.desiredFrames = 3;
    //info.swapchainDescription.format        = ResourceFormat_R8G8B8A8_Unorm;
    //info.swapchainDescription.renderWidth   = configs.renderWidth;
    //info.swapchainDescription.renderHeight  = configs.renderHeight;

    ResultCode result          = RecluseResult_Ok;
    
    result = m_pAdapter->createDevice(info, &m_pDevice);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("Renderer", "Failed to create device!");   
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

    // DebugRenderer Module.
    Plugin* debugPlugin = getPlugin(RendererPluginID_DebugRenderer);
    if (debugPlugin)
    {
        R_DEBUG("Renderer", "Initializing Debug Renderer Plugin");
        debugPlugin->initialize(this);
    }
}


void Renderer::cleanUpModules()
{
    //PreZ::destroy(m_pDevice);

    Plugin* debugPlugin = getPlugin(RendererPluginID_DebugRenderer);
    if (debugPlugin)
    {
        R_DEBUG("Renderer", "Cleaning up Debug Renderer Plugin.");
        debugPlugin->cleanUp(this);
    }
}


VertexBuffer* Renderer::createVertexBuffer(U64 perVertexSzBytes, U64 totalVertices)
{
    ResultCode result          = RecluseResult_Ok;
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
    ResultCode result = RecluseResult_Ok;
    IndexBuffer* pBuffer = new IndexBuffer();
    
    result = pBuffer->initializeIndices(m_pDevice, indexType, totalIndices);

    if (result != RecluseResult_Ok) 
    {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}


ResultCode Renderer::destroyGPUBuffer(GPUBuffer* pBuffer)
{
    ResultCode result = RecluseResult_Ok;

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

    if (renderFlags & Render_ForwardOpaque)
    {
        m_currentCommandKeys[Render_ForwardOpaque].push_back(key.value);
    }

    if (renderFlags & Render_ForwardTransparent)
    {
        m_currentCommandKeys[Render_ForwardTransparent].push_back(key.value);
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
    ResultCode result = RecluseResult_Ok;

    result = pTexture->initialize(this, format, width, height, layers, mips);

    if (result != RecluseResult_Ok) 
    {
        pTexture->destroy(this);
        delete pTexture;
    }
  
    return pTexture;
}


ResultCode Renderer::destroyTexture2D(Texture2D* pTexture)
{
    R_ASSERT(pTexture != NULL);

    pTexture->destroy(this);
    delete pTexture;

    return RecluseResult_Ok;
}


static ResultCode kRendererJob(void* pData)
{
    Renderer* pRenderer                     = Renderer::getMain();
    Mutex renderMutex                       = pRenderer->getMutex();
    U64 threadId                            = getCurrentThreadId();
    F32 counterFrameMs                      = 0.f;

        // Initialize the renderer watch.
    RealtimeTick::initializeWatch(threadId, JobType_Renderer);

    // Initialize module here.
    while (pRenderer->isActive()) 
    {
        ScopedLock lck(renderMutex);
        RealtimeTick::updateWatch(threadId, JobType_Renderer);

        RealtimeTick tick = RealtimeTick::getTick(JobType_Renderer);

        // Render interpolation is required.
        if (pRenderer->isRunning()) 
        {
            const RendererConfigs& renderConfigs    = pRenderer->getCurrentConfigs();
            const F32 desiredFrameRateMs = 1.0f / renderConfigs.maxFrameRate;
            Limiter::limit(desiredFrameRateMs, threadId, JobType_Renderer);
            pRenderer->update(tick.getCurrentTimeS(), tick.delta());
            pRenderer->render();
            pRenderer->present();
        }
    }

    return RecluseResult_Ok;
}


ResultCode Renderer::onInitializeModule(Application* pApp)
{
    m_configLock = createMutex();

    MainThreadLoop::getMessageBus()->addReceiver(
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
            });

    return pApp->loadJobThread(JobType_Renderer, kRendererJob);
}


ResultCode Renderer::onCleanUpModule(Application* pApp)
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


void Renderer::clear()
{
    m_debugDrawFunctions.clear();
}


TemporaryBuffer Renderer::createTemporaryBuffer(const TemporaryBufferDescription& description)
{
    TemporaryBuffer temp = { };
    return (void*)0;
}


ResultCode Renderer::createTemporaryResourcePool(U32 bufferCount)
{
    // First free the temporary resources.
    freeTemporaryResources();

    for (U32 i = 0; i < 2; ++i)
    {
        m_temporaryPools[i].Cs.initialize();
        m_temporaryPools[i].PerFrameAllocator.resize(bufferCount);
        m_temporaryPools[i].PerFramePool.resize(bufferCount);
    }

    R_ASSERT(m_pDevice);

    TemporaryPool& pool = m_temporaryPools[ResourceMemoryUsage_CpuVisible];
    for (U32 i = 0; i < pool.PerFramePool.size(); ++i)
    {
        GraphicsResource* tempResourcePool = nullptr;
        GraphicsResourceDescription desc = { };
        desc.samples = 1;
        desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
        desc.dimension = ResourceDimension_Buffer;
        desc.depthOrArraySize = 1;
        desc.height = 1;
        desc.width = R_MB(64);
        desc.usage = ResourceUsage_ConstantBuffer 
            | ResourceUsage_IndexBuffer 
            | ResourceUsage_VertexBuffer 
            | ResourceUsage_UnorderedAccess 
            | ResourceUsage_CopyDestination
            | ResourceUsage_IndirectBuffer;
        ResultCode result = m_pDevice->createResource(&tempResourcePool, desc, ResourceState_Common);
        R_ASSERT_FORMAT(result == RecluseResult_Ok, "Temp resource creation failed with the error: %d", result);
        pool.PerFramePool[i] = tempResourcePool;
        pool.PerFrameAllocator[i] = new LinearAllocator();
        pool.PerFrameAllocator[i]->initialize(0, desc.width);
    }

    pool = m_temporaryPools[ResourceMemoryUsage_GpuOnly];
    for (U32 i = 0; i < pool.PerFramePool.size(); ++i)
    {
        GraphicsResource* tempResourcePool = nullptr;
        GraphicsResourceDescription desc = { };
        desc.dimension = ResourceDimension_Buffer;
        desc.format = ResourceFormat_Unknown;
        desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        ResultCode result = m_pDevice->createResource(&tempResourcePool, desc, ResourceState_Common);
        R_ASSERT_FORMAT(result == RecluseResult_Ok, "Temp resource creation failed with the error: %d", result);
        pool.PerFramePool[i] = tempResourcePool;
        pool.PerFrameAllocator[i] = new LinearAllocator();
        pool.PerFrameAllocator[i]->initialize(0, desc.width);
    }

    return RecluseResult_Ok;
}


ResultCode Renderer::freeTemporaryResources()
{
    for (U32 i = 0; i < 2; ++i)
    {
        TemporaryPool& pool = m_temporaryPools[i];
        for (U32 i = 0; i < pool.PerFrameAllocator.size(); ++i)
        {
            if (pool.PerFrameAllocator[i])
            {
                pool.PerFrameAllocator[i]->cleanUp();
                delete pool.PerFrameAllocator[i];
            }
            pool.PerFrameAllocator[i] = nullptr;
        }

        for (U32 i = 0; i < pool.PerFramePool.size(); ++i)
        {
            if (pool.PerFramePool[i])
            {
                m_pDevice->destroyResource(pool.PerFramePool[i]);
            }
            pool.PerFramePool[i] = nullptr;
        }

        pool.Cs.release();
    }
    return RecluseResult_Ok;
}
} // Engine
} // Recluse