//

#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include "Recluse/Messaging.hpp"

#include "PreZRenderModule.hpp"
#include "AOVRenderModule.hpp"
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
    sortCommandKeys();

    m_commandList->begin();

        // TODO: Would make more sense to manually transition the resource itself, 
        //       and not the resource view...
        GraphicsResource* pSceneDepth = m_sceneBuffers.pSceneDepth->getResource();
        
        if (pSceneDepth->getCurrentResourceState() != RESOURCE_STATE_DEPTH_STENCIL_WRITE) {
            // Transition the resource.
            ResourceTransition trans = MAKE_RESOURCE_TRANSITION(pSceneDepth, RESOURCE_STATE_DEPTH_STENCIL_WRITE, 0, 1, 0, 1);
            m_commandList->transition(&trans, 1);            
        }

        
        PreZ::generate(m_commandList, m_renderCommands, 
            m_commandKeys[SURFACE_OPAQUE].data(), m_commandKeys[SURFACE_OPAQUE].size());

        AOV::generate(m_commandList, m_renderCommands,
            m_commandKeys[SURFACE_OPAQUE].data(), m_commandKeys[SURFACE_OPAQUE].size());
        

    m_commandList->end();


    QueueSubmit submit      = { };
    submit.numCommandLists  = 1;
    submit.pCommandLists    = &m_commandList;

    m_graphicsQueue->submit(&submit);
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


void Renderer::pushRenderCommand(const RenderCommand& renderCommand)
{
    R_ASSERT(m_renderCommands != NULL);

    // Store mesh commands to be referenced for each draw pass.
    CommandKey key                          = { m_renderCommands->getNumberCommands() };
    SubmeshRenderCommand* pSubMeshCommands  = renderCommand.pSubmeshes;

    for (U32 i = 0; i < renderCommand.numSubMeshCommands; ++i) {
    
        Material* pMaterial             = pSubMeshCommands[i].pMaterial;
        SubMeshRenderFlags submeshFlags = pSubMeshCommands[i].flags;

        // Not material means we don't know how to render this mesh?
        if (!pMaterial) {
            continue;
        }

        SurfaceTypeFlags surface = pMaterial->getSurfaceTypeFlags();

        if (surface & SURFACE_OPAQUE) {
            m_commandKeys[SURFACE_OPAQUE].push_back(key.value);
        }

        if (surface & SURFACE_SHADOWS) {
            m_commandKeys[SURFACE_SHADOWS].push_back(key.value);
        }
    }

    // Push the render command to the last, this will serve as our reference to render in
    // certain render passes.
    m_renderCommands->push(renderCommand);
}
} // Engine
} // Recluse