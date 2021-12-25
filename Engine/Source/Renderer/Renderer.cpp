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

namespace Recluse {
namespace Engine {


void Renderer::initialize(void* windowHandle, const RendererConfigs& configs)
{
    EnableLayerFlags flags  = 0;
    ApplicationInfo info    = { };
    ErrType result          = REC_RESULT_OK;
    m_windowHandle          = windowHandle;

    m_pInstance = GraphicsInstance::createInstance(configs.api);
    
    if (!m_pInstance) {

        R_ERR("Renderer", "Failed to create graphics context, aborting...");

        return;
    }

    info.engineName     = RECLUSE_ENGINE_NAME_STRING;
    info.engineMajor    = RECLUSE_ENGINE_VERSION_MAJOR;
    info.engineMinor    = RECLUSE_ENGINE_VERSION_MINOR;
    info.enginePatch    = RECLUSE_ENGINE_VERSION_PATCH;
    info.appName        = "NoName";

    result = m_pInstance->initialize(info, flags);

    if (result != REC_RESULT_OK) {

        R_ERR("Renderer", "Failed to initialize instance!");        

    }

    std::vector<GraphicsAdapter*> adapters = m_pInstance->getGraphicsAdapters();
    
    determineAdapter(adapters);

    createDevice(configs);

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

    if (result != REC_RESULT_OK) {
    
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

    if (m_pDevice) {
        m_pAdapter->destroyDevice(m_pDevice);
    }
}


void Renderer::present()
{
    ErrType result = m_pSwapchain->present();

    if (result != REC_RESULT_OK) {

        R_WARN("Renderer", "Swapchain present returns with err code: %d", result);

    }
}


void Renderer::render()
{
    sortCommandKeys();

    m_commandList->begin();

        // TODO: Would make more sense to manually transition the resource itself, 
        //       and not the resource view...
        GraphicsResource* pSceneDepth = m_sceneBuffers.pSceneDepth->getResource();
        //GraphicsResource* pSceneAlbedo = m_sceneBuffers.pSceneAlbedo->getResource();
        
        if (pSceneDepth->getCurrentResourceState() != RESOURCE_STATE_DEPTH_STENCIL_WRITE) {
            // Transition the resource.
            ResourceTransition trans        = MAKE_RESOURCE_TRANSITION(pSceneDepth, RESOURCE_STATE_DEPTH_STENCIL_WRITE, 0, 1, 0, 1);
            //ResourceTransition albedoTrans  = MAKE_RESOURCE_TRANSITION(pSceneAlbedo, RESOURCE_STATE_RENDER_TARGET, 0, 1, 0, 1);
            m_commandList->transition(&trans, 1);            
        }

        PreZ::generate(m_commandList, m_renderCommands, 
            m_commandKeys[RENDER_PREZ].data(), 
            m_commandKeys[RENDER_PREZ].size());

        // Re-transition back to read only.
        if (pSceneDepth->getCurrentResourceState() != RESOURCE_STATE_DEPTH_STENCIL_READONLY) {
            ResourceTransition trans = MAKE_RESOURCE_TRANSITION(pSceneDepth, RESOURCE_STATE_DEPTH_STENCIL_READONLY, 0, 1, 0, 1);
            m_commandList->transition(&trans, 1);
        }

        // Asyncronous Queue -> Do Light culling here.
        LightCluster::cullLights(m_commandList);

        AOV::generate(m_commandList, m_renderCommands,
            m_commandKeys[RENDER_GBUFFER].data(), 
            m_commandKeys[RENDER_GBUFFER].size());

        // Deferred rendering combine.
        LightCluster::combineDeferred(m_commandList);

        // Forward pass combine.
        LightCluster::combineForward(m_commandList, 
            m_commandKeys[RENDER_FORWARD_OPAQUE].data(), 
            m_commandKeys[RENDER_FORWARD_OPAQUE].size());

    m_commandList->end();

    // Present.
    m_pSwapchain->present();
    
    resetCommandKeys();
}


void Renderer::determineAdapter(std::vector<GraphicsAdapter*>& adapters)
{
    for (U32 i = 0; i < adapters.size(); ++i) {
        AdapterInfo info            = { };
        AdapterLimits limits        = { };
        GraphicsAdapter* pAdapter   = adapters[i];
        ErrType result              = REC_RESULT_OK;

        result = pAdapter->getAdapterInfo(&info);
    
        if (result != REC_RESULT_OK) {
    
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
    DeviceCreateInfo info   = { };
    info.buffering          = configs.buffering;
    info.winHandle          = m_windowHandle;
    ErrType result          = REC_RESULT_OK;
    
    result = m_pAdapter->createDevice(info, &m_pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Renderer", "Failed to create device!");
        
    }
}


void Renderer::setUpModules()
{
    m_sceneBuffers.pSceneDepth = new Texture2D();
    m_sceneBuffers.pSceneDepth->initialize(this, RESOURCE_FORMAT_D32_FLOAT_S8_UINT, 
        m_rendererConfigs.renderWidth, m_rendererConfigs.renderHeight, 1, 1);

    PreZ::initialize(m_pDevice, &m_sceneBuffers);
}


void Renderer::cleanUpModules()
{
    PreZ::destroy(m_pDevice);
}


VertexBuffer* Renderer::createVertexBuffer(U64 perVertexSzBytes, U64 totalVertices)
{
    ErrType result          = REC_RESULT_OK;
    VertexBuffer* pBuffer   = new VertexBuffer();

    result = pBuffer->initializeVertices(m_pDevice, perVertexSzBytes, totalVertices);

    if (result != REC_RESULT_OK) {
    
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

    if (result != REC_RESULT_OK) {
    
        delete pBuffer;
        pBuffer = nullptr;
    
    }

    return pBuffer;
}


ErrType Renderer::destroyGPUBuffer(GPUBuffer* pBuffer)
{
    ErrType result = REC_RESULT_OK;

    if (!pBuffer) {

        return REC_RESULT_FAILED;

    }

    result = pBuffer->destroy();
    delete pBuffer;

    return result;
}


void Renderer::resetCommandKeys()
{
    for (auto& cmdLists : m_commandKeys) {
    
        cmdLists.second.clear();
    
    }
}


void Renderer::sortCommandKeys()
{
    struct Cmp { 
        bool operator()(const U64 a, const U64 b) const { 
            return a < b;
        }
    } pred;

    for (auto& cmdLists : m_commandKeys) {
    
        std::vector<U64>& list = cmdLists.second;
        std::sort(list.begin(), list.end(), pred);
    
    }
}


void Renderer::pushRenderCommand(const RenderCommand& renderCommand, RenderPassTypeFlags renderFlags)
{
    R_ASSERT(m_renderCommands != NULL);

    // Store mesh commands to be referenced for each draw pass.
    CommandKey key                  = { m_renderCommands->getNumberCommands() };

    if (renderFlags & RENDER_PREZ) {
        m_commandKeys[RENDER_PREZ].push_back(key.value);
    }

    if (renderFlags & RENDER_GBUFFER) {
        m_commandKeys[RENDER_GBUFFER].push_back(key.value);
    }

    if (renderFlags & RENDER_SHADOW) {
        m_commandKeys[RENDER_SHADOW].push_back(key.value);
    }

    // Mesh is treated as particles.
    if (renderFlags & RENDER_PARTICLE) {
        m_commandKeys[RENDER_PARTICLE].push_back(key.value);
    }

    // Push the render command to the last, this will serve as our reference to render in
    // certain render passes.
    m_renderCommands->push(renderCommand);
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

    m_renderCommands = new RenderCommandList();
    m_renderCommands->initialize();
}


void Renderer::freeSceneBuffers()
{

    if (m_sceneBuffers.pDepthStencilView) {
        m_sceneBuffers.pDepthStencilView->destroy(this);
        delete m_sceneBuffers.pDepthStencilView;
        m_sceneBuffers.pDepthStencilView = nullptr;
    }

    if (m_sceneBuffers.pSceneDepth) {
    
        destroyTexture2D(m_sceneBuffers.pSceneDepth);
        m_sceneBuffers.pSceneDepth = nullptr;

    }

    if (m_renderCommands) {
    
        m_renderCommands->destroy();
        delete m_renderCommands;
        m_renderCommands = nullptr;
    
    }
}


Texture2D* Renderer::createTexture2D(U32 width, U32 height, U32 mips, U32 layers, ResourceFormat format)
{
    Texture2D* pTexture = new Texture2D();
    ErrType result = REC_RESULT_OK;

    result = pTexture->initialize(this, format, width, height, layers, mips);

    if (result != REC_RESULT_OK) {
    
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
    Renderer* pRenderer = Renderer::getMain();

    // Initialize module here.
    while (pRenderer->isActive()) {

        // Render interpolation is required.

        if (pRenderer->isRunning()) {
            pRenderer->render();
            pRenderer->present();
        }
    }

    return REC_RESULT_OK;
}


ErrType Renderer::onInitializeModule(Application* pApp)
{
    {
        Renderer* pRenderer = Renderer::getMain();
        Window* pWindow = pApp->getWindow();
        RendererConfigs configs = { };
        pRenderer->initialize(pWindow->getNativeHandle(), configs);
    }

    MainThreadLoop::getMessageBus()->addReceiver(
        "Renderer", [=] (AMessage* pMsg) -> void { 
        std::string ev = pMsg->getEvent();
        if (ev.compare("Renderer") == 0) {
            R_DEBUG("Renderer", "Received message!");
            RenderMessage* pJobMessage = static_cast<RenderMessage*>(pMsg);
            Renderer* pRenderer = Renderer::getMain();
            if (pRenderer->isActive()) {
                // Handle the message.
                switch (pJobMessage->req) {
                    case RenderMessage::RESUME:
                        pRenderer->enableRunning(true);
                        break;
                    case RenderMessage::PAUSE:
                        pRenderer->enableRunning(false);
                        break;
                    case RenderMessage::SHUTDOWN: 
                    {
                        pRenderer->enableRunning(false);
                        pRenderer->cleanUpModule(pJobMessage->pApp);
                        break;
                    }
                    case RenderMessage::CHANGE_CONFIG:
                    case RenderMessage::SCENE_UPDATE:
                    default:
                        break;
                }
            }
        }
    });

    return pApp->loadJobThread(JOB_TYPE_RENDERER, kRendererJob);
}


ErrType Renderer::onCleanUpModule(Application* pApp)
{
    Renderer::getMain()->cleanUp();
    return REC_RESULT_OK;
}


void Renderer::destroyDevice()
{
    if (m_pDevice) {
        m_pAdapter->destroyDevice(m_pDevice);
        m_pDevice = nullptr;
    } R_DEBUG_WRAP( else { 
        R_WARN("Renderer", "No such graphics device exists for this renderer."); 
    })
}
} // Engine
} // Recluse