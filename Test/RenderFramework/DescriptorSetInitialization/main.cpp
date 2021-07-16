//
#include "Recluse/Messaging.hpp"
#include "Recluse/RealtimeTick.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/CommandQueue.hpp"
#include "Recluse/Graphics/RenderPass.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

using namespace Recluse;

struct VertexData {
    F32 position[4];
    F32 normal[3];
    F32 pad0;
    F32 texcoord0[2];
    F32 texcoord1[2];
};

void updateResource(GraphicsResource* pResource)
{
    VertexData dat      = { };
    dat.position[0]     = 0.1f;
    dat.position[1]     = 0.2f;
    dat.position[2]     = 0.3f;
    dat.position[3]     = 0.4f;

    dat.normal[0]       = 1.0f;
    dat.normal[1]       = -1.0f;
    dat.normal[2]       = 0.5f;
    
    dat.texcoord0[0]    = 45.0f;
    dat.texcoord0[1]    = 0.3f;
    
    dat.texcoord1[0]    = 6.0f;
    dat.texcoord1[1]    = 10.0f;

    MapRange mapRange   = { };
    void* ptr           = nullptr;
    ErrType result      = pResource->map(&ptr, &mapRange);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to map to resource!");
    
    }

    memcpy(ptr, &dat, sizeof(dat));
}


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();

    GraphicsContext* pContext       = nullptr;
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    Window* pWindow                 = nullptr;
    GraphicsCommandList* pList      = nullptr;
    DescriptorSetLayout* pLayout    = nullptr;
    GraphicsResource* pData         = nullptr;
    GraphicsQueue* pQueue           = nullptr;
    DescriptorSet*  pSet            = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    RenderPass* pRenderPass         = nullptr;
    ErrType result                  = REC_RESULT_OK;

    pWindow = Window::create(u8"DescriptorSetInitialization", 0, 0, 1024, 1024);

    pContext = GraphicsContext::createContext(GRAPHICS_API_VULKAN);

    if (!pContext) {

        R_ERR("TEST", "Failed to create graphics context!");

    }

    {
        ApplicationInfo appInfo = { };
        appInfo.appMajor = 0;
        appInfo.appMinor = 0;
        appInfo.appPatch = 0;
        appInfo.appName = "DescriptorSetInitialization";
        appInfo.engineMajor = 0;
        appInfo.engineMinor = 0;
        appInfo.engineName = "DescriptorSetInitialization";

        EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

        result = pContext->initialize(appInfo, flags);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to initialize context!");    

    }

    std::vector<GraphicsAdapter*> adapters = pContext->getGraphicsAdapters();
    pAdapter = adapters[0];

    {
        DeviceCreateInfo info = { };
        info.buffering = 2;
        info.winHandle = pWindow->getNativeHandle();
        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create device from adapter!");
    
    }

    {
        MemoryReserveDesc mem = { };
        mem.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY] = 1 * R_1MB;
        mem.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY] = 1 * R_1MB;
        mem.bufferPools[RESOURCE_MEMORY_USAGE_CPU_TO_GPU] = 1 * R_1KB;
        mem.bufferPools[RESOURCE_MEMORY_USAGE_GPU_TO_CPU] = 1 * R_1KB;
        mem.texturePoolGPUOnly = 1 * R_1MB;
        result = pDevice->reserveMemory(mem);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to reserve memory!");
    
    }

    result = pDevice->createCommandQueue(&pQueue, QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_PRESENT);

    if (result != REC_RESULT_OK) {

        R_ERR("TEST", "Failed to create device queue!");

    }

    {
        SwapchainCreateDescription info = { };
        info.buffering = FRAME_BUFFERING_DOUBLE;
        info.desiredFrames = 2;
        info.pBackbufferQueue = pQueue;
        info.renderWidth = 1024;
        info.renderHeight = 1024;
        result = pDevice->createSwapchain(&pSwapchain, info);
    }

    if (result != REC_RESULT_OK) {

        R_ERR("TEST", "FAILED TO CREATE SWAPCHAIN!!");    

    }

    result = pDevice->createCommandList(&pList, QUEUE_TYPE_GRAPHICS);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create command list!");
    
    }

    {
        GraphicsResourceDescription desc = { };
        desc.usage = RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.dimension = RESOURCE_DIMENSION_BUFFER;
        desc.width = sizeof(VertexData);
        desc.depth = 1;
        desc.height = 1;
        desc.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_ONLY;
        desc.samples = 1;
        result = pDevice->createResource(&pData, desc);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create resource!");
    
    }

    updateResource(pData);

    {
        DescriptorBindDesc bindLayout = { };
        DescriptorSetLayoutDesc desc = { };
        desc.pDescriptorBinds = &bindLayout;
        desc.numDescriptorBinds = 1;

        bindLayout.binding = 0;
        bindLayout.bindType = DESCRIPTOR_CONSTANT_BUFFER;
        bindLayout.numDescriptors = 1;
        bindLayout.shaderStages = SHADER_TYPE_FRAGEMENT | SHADER_TYPE_VERTEX;

        result = pDevice->createDescriptorSetLayout(&pLayout, desc);
        
    }
    
    if (result != REC_RESULT_OK) {

        R_ERR("TEST", "Failed to create descriptor set layout!");
    
    }

    result = pDevice->createDescriptorSet(&pSet, pLayout);

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create descriptor set!");
    
    }

    {
        DescriptorSetBind bind = { };
        bind.binding = 0;
        bind.bindType = DESCRIPTOR_CONSTANT_BUFFER;
        bind.descriptorCount = 1;
        bind.cb.buffer = pData;
        bind.cb.offset = 0;
        bind.cb.sizeBytes = sizeof(VertexData);
        result = pSet->update(&bind, 1);
    }
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to update descriptor set!");
    
    }
    
    pWindow->open();

    // TODO: Still needs more work!!

    // Enter game loop.
    R_TRACE("TEST", "Entering game loop...");

    while (!pWindow->shouldClose()) {
        pList->begin();
            //pList->setRenderPass(pRenderPass);
        pList->end();

        pSwapchain->present();
        pollEvents();
    }

    R_TRACE("TEST", "Exited game loop...");
    pQueue->wait();    

    pDevice->destroyDescriptorSetLayout(pLayout);
    pDevice->destroyDescriptorSet(pSet);
    pDevice->destroyResource(pData);
    pDevice->destroyCommandList(pList);
    pDevice->destroySwapchain(pSwapchain);
    pDevice->destroyCommandQueue(pQueue);    
    pAdapter->destroyDevice(pDevice);
    Window::destroy(pWindow);
    GraphicsContext::destroyContext(pContext);
    Log::destroyLoggingSystem();
    return 0;
}