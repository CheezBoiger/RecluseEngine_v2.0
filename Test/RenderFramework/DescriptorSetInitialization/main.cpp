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
#include "Recluse/System/Limiter.hpp"
#include "Recluse/Math/MathCommons.hpp"

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
    ResultCode result      = pResource->map(&ptr, &mapRange);
    
    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to map to resource!");
    
    }

    memcpy(ptr, &dat, sizeof(dat));
    pResource->unmap(&mapRange);
}


int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    GraphicsInstance* pInstance     = nullptr;
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsContext* pContext       = nullptr;
    Window* pWindow                 = nullptr;
    GraphicsResource* pData         = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsContext* context        = nullptr;
    std::vector<RenderPass*> renderPasses;
    ResultCode result                  = RecluseResult_Ok;

    pWindow = Window::create(u8"DescriptorSetInitialization", 0, 0, 1024, 1024);

    pInstance = GraphicsInstance::createInstance(GraphicsApi_Vulkan);

    if (!pInstance) {

        R_ERROR("TEST", "Failed to create graphics context!");

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

        LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation | LayerFeatureFlag_Raytracing;

        result = pInstance->initialize(appInfo, flags);
    }

    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to initialize context!");    

    }

    std::vector<GraphicsAdapter*> adapters = pInstance->getGraphicsAdapters();
    pAdapter = adapters[0];

    {
        DeviceCreateInfo info = { };
        info.winHandle = pWindow->getNativeHandle();
        info.swapchainDescription.buffering = FrameBuffering_Triple;
        info.swapchainDescription.desiredFrames = 2;
        info.swapchainDescription.renderWidth = 1024;
        info.swapchainDescription.renderHeight = 1024;
        info.swapchainDescription.format = ResourceFormat_B8G8R8A8_Unorm;
        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to create device from adapter!");
    
    }

    pContext = pDevice->createContext();

    {
        MemoryReserveDescription mem = { };
        mem.bufferPools[ResourceMemoryUsage_CpuOnly] = 1 * R_1MB;
        mem.bufferPools[ResourceMemoryUsage_GpuOnly] = 1 * R_1MB;
        mem.bufferPools[ResourceMemoryUsage_CpuToGpu] = 1 * R_1KB;
        mem.bufferPools[ResourceMemoryUsage_GpuToCpu] = 1 * R_1KB;
        mem.texturePoolGPUOnly = 1 * R_1MB;
        result = pDevice->reserveMemory(mem);
    }

    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to reserve memory!");
    
    }

    {
        GraphicsResourceDescription desc = { };
        desc.usage = ResourceUsage_ConstantBuffer;
        desc.dimension = ResourceDimension_Buffer;
        desc.width = sizeof(VertexData);
        desc.depthOrArraySize = 1;
        desc.height = 1;
        desc.memoryUsage = ResourceMemoryUsage_CpuOnly;
        desc.samples = 1;
        result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
    }

    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to create resource!");
    
    }

    updateResource(pData);

    //{
    //    DescriptorBindDesc bindLayout = { };
    //    DescriptorSetLayoutDesc desc = { };
    //    desc.pDescriptorBinds = &bindLayout;
    //    desc.numDescriptorBinds = 1;

    //    bindLayout.binding = 0;
    //    bindLayout.bindType = DescriptorBindType_ConstantBuffer;
    //    bindLayout.numDescriptors = 1;
    //    bindLayout.shaderStages = ShaderType_Fragment | ShaderType_Vertex;

    //    result = pDevice->createDescriptorSetLayout(&pLayout, desc);
    //    
    //}
    //
    //if (result != RecluseResult_Ok) {

    //    R_ERR("TEST", "Failed to create descriptor set layout!");
    //
    //}

    //result = pDevice->createDescriptorSet(&pSet, pLayout);

    //if (result != RecluseResult_Ok) {
    //
    //    R_ERR("TEST", "Failed to create descriptor set!");
    //
    //}

    //{
    //    DescriptorSetBind bind = { };
    //    bind.binding = 0;
    //    bind.bindType = DescriptorBindType_ConstantBuffer;
    //    bind.descriptorCount = 1;
    //    bind.cb.buffer = pData;
    //    bind.cb.offset = 0;
    //    bind.cb.sizeBytes = sizeof(VertexData);
    //    result = pSet->update(&bind, 1);
    //}
    //
    //if (result != RecluseResult_Ok) {
    //
    //    R_ERR("TEST", "Failed to update descriptor set!");
    //
    //}

    //pSwapchain = pDevice->getSwapchain();

    //{
    //    renderPasses.resize(pSwapchain->getDesc().desiredFrames);
    //    RenderPassDesc desc = { };
    //    desc.numRenderTargets = 1;
    //    desc.width = pSwapchain->getDesc().renderWidth;
    //    desc.height = pSwapchain->getDesc().renderHeight;
    //    for (U32 i = 0; i < pSwapchain->getDesc().desiredFrames; ++i) 
    //    {        
    //        desc.ppRenderTargetViews[0] = pSwapchain->getFrameView(i);
    //        result = pDevice->createRenderPass(&renderPasses[i], desc);

    //        if (result != RecluseResult_Ok) 
    //        {
    //            R_ERR("TEST", "Failed to create render pass for frame view: %d", i);
    //            
    //        }
    //    }
    //}
    
    pWindow->open();

    // TODO: Still needs more work!!

    // Enter game loop.
    R_TRACE("TEST", "Entering game loop...");

    pSwapchain = pDevice->getSwapchain();
    context = pContext;
    context->setBuffers(3);
    F32 index = 200;
    F32 counterFps = 0.f;
    F32 desiredFps = 1.f / 256.f;
    while (!pWindow->shouldClose()) 
    {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick   = RealtimeTick::getTick(0);
        F32 ms              = Limiter::limit(desiredFps, 1ull, 0);
        //R_TRACE("TEST", "FPS: %f", 1.0f / tick.getDeltaTimeS());
        F32 color[]         = { 0.0f, 1.0f, 0.0f, 1.0f };
        F32 color2[]        = { 0.0f, 0.0f, 1.0f, 1.0f };
        index               = Math::clamp(index - 100.f * ms, 0.f, 200.0f);
        Rect rect           = { (F32)index, 200.f, 1024.f/2.f, 1024.f/2.f };
        Rect rect2          = { 0.f, 0.f, 1024.f, 1024.f };

        context->begin();
            GraphicsResourceView* pView = pSwapchain->getFrameView(pSwapchain->getCurrentFrameIndex());
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->bindRenderTargets(1, &pView, nullptr);
            context->clearRenderTarget(0, color2, rect2);
            context->clearRenderTarget(0, color, rect);
            context->bindRenderTargets(1, &pView, nullptr);
            context->clearRenderTarget(0, color2, rect2);
            context->clearRenderTarget(0, color, rect);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
        context->end();
        R_FATAL_ERROR("TEST", "%f Fps", 1.0f / ms);
        pSwapchain->present();
        pollEvents();
    }

    R_TRACE("TEST", "Exited game loop...");

    pContext->wait();    
    pDevice->destroyResource(pData); 
    pDevice->releaseContext(context);
    pAdapter->destroyDevice(pDevice);
    Window::destroy(pWindow);
    GraphicsInstance::destroyInstance(pInstance);
    Log::destroyLoggingSystem();
    return 0;
}