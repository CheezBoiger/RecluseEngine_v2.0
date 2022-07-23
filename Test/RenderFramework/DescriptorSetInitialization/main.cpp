//
#include "Recluse/Messaging.hpp"
#include "Recluse/Time.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
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

    MapRange mapRange       = { };
    mapRange.offsetBytes    = 0;
    mapRange.sizeBytes      = sizeof(VertexData);

    void* ptr           = nullptr;
    ErrType result      = pResource->map(&ptr, &mapRange);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to map to resource!");
    
    }

    memcpy(ptr, &dat, sizeof(dat));
    pResource->unmap(&mapRange);
}


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);

    GraphicsInstance* pInstance       = nullptr;
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    Window* pWindow                 = nullptr;
    GraphicsCommandList* pList      = nullptr;
    DescriptorSetLayout* pLayout    = nullptr;
    GraphicsResource* pData         = nullptr;
    DescriptorSet*  pSet            = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    std::vector<RenderPass*> renderPasses;
    ErrType result                  = REC_RESULT_OK;

    pWindow = Window::create(u8"DescriptorSetInitialization", 0, 0, 1024, 1024);

    pInstance = GraphicsInstance::createInstance(GRAPHICS_API_VULKAN);

    if (!pInstance) {

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

        result = pInstance->initialize(appInfo, flags);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to initialize context!");    

    }

    std::vector<GraphicsAdapter*> adapters = pInstance->getGraphicsAdapters();
    pAdapter = adapters[0];

    {
        DeviceCreateInfo info = { };
        info.buffering = 2;
        info.winHandle = pWindow->getNativeHandle();
        info.swapchainDescription.buffering = FRAME_BUFFERING_TRIPLE;
        info.swapchainDescription.desiredFrames = 2;
        info.swapchainDescription.renderWidth = 1024;
        info.swapchainDescription.renderHeight = 1024;
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

    {
        SwapchainCreateDescription info = { };
    }

    if (result != REC_RESULT_OK) {

        R_ERR("TEST", "FAILED TO CREATE SWAPCHAIN!!");    

    }

    pList = pDevice->getCommandList();

    {
        GraphicsResourceDescription desc = { };
        desc.usage = RESOURCE_USAGE_CONSTANT_BUFFER;
        desc.dimension = RESOURCE_DIMENSION_BUFFER;
        desc.width = sizeof(VertexData);
        desc.depth = 1;
        desc.height = 1;
        desc.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_ONLY;
        desc.samples = 1;
        result = pDevice->createResource(&pData, desc, RESOURCE_STATE_VERTEX_AND_CONST_BUFFER);
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
        bindLayout.shaderStages = SHADER_TYPE_FRAGMENT | SHADER_TYPE_VERTEX;

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

    pSwapchain = pDevice->getSwapchain();

    {
        renderPasses.resize(pSwapchain->getDesc().desiredFrames);
        RenderPassDesc desc = { };
        desc.numRenderTargets = 1;
        desc.width = pSwapchain->getDesc().renderWidth;
        desc.height = pSwapchain->getDesc().renderHeight;
        for (U32 i = 0; i < pSwapchain->getDesc().desiredFrames; ++i) {        
            desc.ppRenderTargetViews[0] = pSwapchain->getFrameView(i);
            result = pDevice->createRenderPass(&renderPasses[i], desc);

            if (result != REC_RESULT_OK) {
        
                R_ERR("TEST", "Failed to create render pass for frame view: %d", i);
                
            }
        }
    }
    
    pWindow->open();

    // TODO: Still needs more work!!

    // Enter game loop.
    R_TRACE("TEST", "Entering game loop...");

    F32 index = 200;
    F32 counterFps = 0.f;
    F32 desiredFps = 1.f / 144.f;
    while (!pWindow->shouldClose()) {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        //R_TRACE("TEST", "FPS: %f", 1.0f / tick.getDeltaTimeS());
        F32 color[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        F32 color2[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        if (index != 0) index -= 50.f * tick.getDeltaTimeS();
        Rect rect = { 200.f + (F32)index, 200.f, 1024.f/2.f, 1024.f/2.f };
        Rect rect2 = { 0.f, 0.f, 1024.f, 1024.f };
        pList->begin();
            ResourceTransition trans = MAKE_RESOURCE_TRANSITION(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), RESOURCE_STATE_RENDER_TARGET, 0, 1, 0, 1);
            pList->transition(&trans, 1);
            pList->setRenderPass(renderPasses[pSwapchain->getCurrentFrameIndex()]);
            pList->clearRenderTarget(0, color2, rect2);
            pList->clearRenderTarget(0, color, rect);
            pList->setRenderPass(renderPasses[pSwapchain->getCurrentFrameIndex()]);
            pList->clearRenderTarget(0, color2, rect2);
            pList->clearRenderTarget(0, color, rect);

            GraphicsResourceView* pView = pSwapchain->getFrameView(pSwapchain->getCurrentFrameIndex());
            pList->transition(&trans, 1);
            pList->transition(&trans, 1);
        pList->end();

        F32 deltaFrameRate = 1.0f / tick.getDeltaTimeS();
        counterFps += tick.getDeltaTimeS();

        GraphicsSwapchain::PresentConfig conf = GraphicsSwapchain::DELAY_PRESENT;
        if (counterFps >= desiredFps)
        {
            R_VERBOSE("Test", "Frame Rate: %f fps, time: %f", 1.0f / counterFps, tick.getCurrentTimeS());
            counterFps = 0.f;
            conf = GraphicsSwapchain::NORMAL_PRESENT;
        }

        pSwapchain->present(conf);
        pollEvents();
    }

    R_TRACE("TEST", "Exited game loop...");
    pDevice->wait();    

    for (U32 i = 0; i < pSwapchain->getDesc().desiredFrames; ++i) {
        pDevice->destroyRenderPass(renderPasses[i]);
    }

    pDevice->destroyDescriptorSetLayout(pLayout);
    pDevice->destroyDescriptorSet(pSet);
    pDevice->destroyResource(pData); 
    pAdapter->destroyDevice(pDevice);
    Window::destroy(pWindow);
    GraphicsInstance::destroyInstance(pInstance);
    Log::destroyLoggingSystem();
    return 0;
}