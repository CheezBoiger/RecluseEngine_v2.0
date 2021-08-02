//

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include "Recluse/Messaging.hpp"

#include "PreZRenderModule.hpp"

namespace Recluse {
namespace Engine {


void Renderer::initialize(void* windowHandle, const RendererConfigs& configs)
{
    EnableLayerFlags flags  = 0;
    ApplicationInfo info    = { };
    ErrType result          = REC_RESULT_OK;
    m_windowHandle          = windowHandle;

    m_pContext = GraphicsContext::createContext(configs.api);
    
    if (!m_pContext) {

        R_ERR("Renderer", "Failed to create graphics context, aborting...");

        return;
    }

    info.engineName     = RECLUSE_ENGINE_NAME_STRING;
    info.engineMajor    = RECLUSE_ENGINE_VERSION_MAJOR;
    info.engineMinor    = RECLUSE_ENGINE_VERSION_MINOR;
    info.enginePatch    = RECLUSE_ENGINE_VERSION_PATCH;
    info.appName        = "NoName";

    result = m_pContext->initialize(info, flags);

    if (result != REC_RESULT_OK) {

        R_ERR("Renderer", "Failed to initialize context!");        

    }

    std::vector<GraphicsAdapter*> adapters = m_pContext->getGraphicsAdapters();
    
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

    setUpModules();
}


void Renderer::cleanUp()
{
    if (m_graphicsQueue) {
        // Wait for the queue to finish all work, before destroying things...    
        m_graphicsQueue->wait();

    }

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

    PreZ::generate(m_commandList);
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
    U64 totalSzBytes        = perVertexSzBytes * totalVertices;

    result = pBuffer->initialize(m_pDevice, totalSzBytes, RESOURCE_USAGE_VERTEX_BUFFER);

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
} // Engine
} // Recluse