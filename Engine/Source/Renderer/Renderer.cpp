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


void Renderer::initialize(void* windowHandle, const RendererConfigs& configs)
{
    R_ASSERT_MSG(configs.buffering >= 1, "Must at least be one buffer count!");
    m_newRendererConfigs = m_currentRendererConfigs = configs;

    m_configLock = createMutex();

    EnableLayerFlags flags  = 0;
    ApplicationInfo info    = { };
    ErrType result          = REC_RESULT_OK;
    m_windowHandle          = windowHandle;

    m_pInstance = GraphicsInstance::createInstance(configs.api);
    
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

    if (result != REC_RESULT_OK) 
    {
        R_ERR("Renderer", "Failed to initialize instance!");        
    }

    std::vector<GraphicsAdapter*> adapters = m_pInstance->getGraphicsAdapters();
    
    determineAdapter(adapters);

    createDevice(configs);
    m_pSwapchain = m_pDevice->getSwapchain();

    {
        MemoryReserveDesc reserveDesc = { };
        reserveDesc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY]     = R_1MB * 32ull;
        reserveDesc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_TO_GPU]   = R_1MB * 16ull;
        reserveDesc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_TO_CPU]   = R_1MB * 16ull;
        reserveDesc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY]     = R_1MB * 512ull;
        reserveDesc.texturePoolGPUOnly                              = R_1GB * 1ull;

        // Memory reserves for engine.
        result = m_pDevice->reserveMemory(reserveDesc);
    }

    if (result != REC_RESULT_OK) 
    {
        R_ERR("Renderer", "ReserveMemory call completed with result: %d", result);
    }

    m_commandList = m_pDevice->getCommandList();

    allocateSceneBuffers(configs);

    setUpModules();
}


void Renderer::cleanUp()
{
    freeSceneBuffers();

    // Clean up all modules, as well as resources handled by them...
    cleanUpModules();

    destroyMutex(m_configLock);

    if (m_pDevice) 
    {
        m_pAdapter->destroyDevice(m_pDevice);
    }
}


void Renderer::present(Bool delayPresent)
{
    ErrType result = m_pSwapchain->present(delayPresent ? GraphicsSwapchain::DELAY_PRESENT : GraphicsSwapchain::NORMAL_PRESENT);

    if (result != REC_RESULT_OK) 
    {
        R_WARN("Renderer", "Swapchain present returns with err code: %d", result);
    }
}


void Renderer::render()
{
    sortCommandKeys();
    m_commandList->begin();
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
    m_commandList->end();
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
        ErrType result              = REC_RESULT_OK;

        result = pAdapter->getAdapterInfo(&info);
    
        if (result != REC_RESULT_OK) 
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
    info.buffering                          = configs.buffering;
    info.winHandle                          = m_windowHandle;
    info.swapchainDescription.buffering     = FRAME_BUFFERING_SINGLE;
    info.swapchainDescription.desiredFrames = 3;
    info.swapchainDescription.format        = RESOURCE_FORMAT_R8G8B8A8_UNORM;
    info.swapchainDescription.renderWidth   = configs.renderWidth;
    info.swapchainDescription.renderHeight  = configs.renderHeight;

    ErrType result          = REC_RESULT_OK;
    
    result = m_pAdapter->createDevice(info, &m_pDevice);

    if (result != REC_RESULT_OK) 
    {
        R_ERR("Renderer", "Failed to create device!");   
    }
}


void Renderer::setUpModules()
{
    m_sceneBuffers.pSceneDepth = new Texture2D();
    m_sceneBuffers.pSceneDepth->initialize
                                    (
                                        this, 
                                        RESOURCE_FORMAT_D32_FLOAT_S8_UINT, 
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
    ErrType result          = REC_RESULT_OK;
    VertexBuffer* pBuffer   = new VertexBuffer();

    result = pBuffer->initializeVertices(m_pDevice, perVertexSzBytes, totalVertices);

    if (result != REC_RESULT_OK) 
    {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}


IndexBuffer* Renderer::createIndexBuffer(IndexType indexType, U64 totalIndices)
{
    ErrType result = REC_RESULT_OK;
    IndexBuffer* pBuffer = new IndexBuffer();
    
    result = pBuffer->initializeIndices(m_pDevice, indexType, totalIndices);

    if (result != REC_RESULT_OK) 
    {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}


ErrType Renderer::destroyGPUBuffer(GPUBuffer* pBuffer)
{
    ErrType result = REC_RESULT_OK;

    if (!pBuffer) 
    {
        return REC_RESULT_FAILED;
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

    if (renderFlags & RENDER_PREZ) 
    {
        m_currentCommandKeys[RENDER_PREZ].push_back(key.value);
    }

    if (renderFlags & RENDER_GBUFFER) 
    {
        m_currentCommandKeys[RENDER_GBUFFER].push_back(key.value);
    }

    if (renderFlags & RENDER_SHADOW) 
    {
        m_currentCommandKeys[RENDER_SHADOW].push_back(key.value);
    }

    // Mesh is treated as particles.
    if (renderFlags & RENDER_PARTICLE) 
    {
        m_currentCommandKeys[RENDER_PARTICLE].push_back(key.value);
    }

    // Push the render command to the last, this will serve as our reference to render in
    // certain render passes.
    m_currentRenderCommands->push(renderCommand);
}


void Renderer::allocateSceneBuffers(const RendererConfigs& configs)
{
    U32 width = configs.renderWidth;
    U32 height = configs.renderHeight;
    m_sceneBuffers.pSceneDepth = createTexture2D(width, height, 1, 1, RESOURCE_FORMAT_D24_UNORM_S8_UINT);

    ResourceViewDesc viewDesc = { };
    viewDesc.dimension = RESOURCE_VIEW_DIMENSION_2D;
    viewDesc.format = RESOURCE_FORMAT_D24_UNORM_S8_UINT;
    viewDesc.layerCount = 1;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.baseMipLevel = 0;
    viewDesc.type = RESOURCE_VIEW_TYPE_DEPTH_STENCIL;

    m_sceneBuffers.pDepthStencilView = new TextureView();
    m_sceneBuffers.pDepthStencilView->initialize(this, m_sceneBuffers.pSceneDepth, viewDesc);

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

    if (m_sceneBuffers.pDepthStencilView) 
    {
        m_sceneBuffers.pDepthStencilView->destroy(this);
        delete m_sceneBuffers.pDepthStencilView;
        m_sceneBuffers.pDepthStencilView = nullptr;
    }

    if (m_sceneBuffers.pSceneDepth) 
    {
    
        destroyTexture2D(m_sceneBuffers.pSceneDepth);
        m_sceneBuffers.pSceneDepth = nullptr;

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
    ErrType result = REC_RESULT_OK;

    result = pTexture->initialize(this, format, width, height, layers, mips);

    if (result != REC_RESULT_OK) 
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

    return REC_RESULT_OK;
}


static ErrType kRendererJob(void* pData)
{
    Renderer* pRenderer                     = Renderer::getMain();
    Mutex renderMutex                       = pRenderer->getMutex();
    U64 threadId                            = getCurrentThreadId();
    F32 counterFrameMs                      = 0.f;
    const RendererConfigs& renderConfigs    = pRenderer->getCurrentConfigs();

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
            pRenderer->update(tick.getCurrentTimeS(), tick.getDeltaTimeS());
            pRenderer->render();

            // Check if we need to limit our frame rate.
            GraphicsSwapchain::PresentConfig presentConfig = GraphicsSwapchain::DELAY_PRESENT;

            if (renderConfigs.maxFrameRate > 0)
            {
                const F32 desiredFramesMs = 1.0f / renderConfigs.maxFrameRate;
                if (counterFrameMs >= desiredFramesMs)
                {
                    counterFrameMs = 0.f;
                    presentConfig = GraphicsSwapchain::NORMAL_PRESENT;
                }
            }
            else
            {
                // Not framerate limit, so we can present as fast as we can.
                presentConfig = GraphicsSwapchain::NORMAL_PRESENT;
            }

            pRenderer->present(presentConfig);
        }
    }

    return REC_RESULT_OK;
}


ErrType Renderer::onInitializeModule(Application* pApp)
{
    {
        Window* pWindow = pApp->getWindow();
        RendererConfigs configs = { };
        configs.buffering = 1;
        configs.api = GRAPHICS_API_VULKAN;
        configs.renderWidth = 800;
        configs.renderHeight = 600;
        configs.maxFrameRate = 60.0f;

        initialize(pWindow->getNativeHandle(), configs);
    }

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
                            case RenderEvent_RESUME:
                                enableRunning(true);
                                break;

                            case RenderEvent_PAUSE:
                                enableRunning(false);
                                break;

                            case RenderEvent_SHUTDOWN: 
                            {
                                enableRunning(false);
                                cleanUpModule(MainThreadLoop::getApp());
                                break;
                            }

                            case RenderEvent_CONFIGURE_RENDERER:
                            {
                                recreate();
                                break;
                            }

                            case RenderEvent_SCENE_UPDATE:
                                break;

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
    return REC_RESULT_OK;
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