﻿//
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
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Math/Bounds2D.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Matrix43.hpp"
#include "Recluse/System/KeyboardInput.hpp"
#include <random>

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

    pWindow = Window::create(u8"DescriptorSetInitialization", 0, 0, 1024, 1024, ScreenMode_WindowBorderless);
    pWindow->setToCenter();

    pInstance = GraphicsInstance::create(GraphicsApi_Direct3D12);

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
        DeviceCreateInfo info = { true };
        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to create device from adapter!");
    
    }

    pContext = pDevice->createContext();

    {
        MemoryReserveDescription mem = { };
        mem.bufferPools[ResourceMemoryUsage_CpuVisible] = 1 * R_1MB;
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
        desc.mipLevels = 1;
        desc.memoryUsage = ResourceMemoryUsage_CpuVisible;
        desc.samples = 1;
        //result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
    }

    if (result != RecluseResult_Ok) {
    
        R_ERROR("TEST", "Failed to create resource!");
    
    }

    //updateResource(pData);

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
    
    pWindow->show();

    // TODO: Still needs more work!!

    // Enter game loop.
    R_TRACE("TEST", "Entering game loop...");

    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.buffering = FrameBuffering_Triple;
    swapchainDescription.desiredFrames = 2;
    swapchainDescription.renderWidth = 1024;
    swapchainDescription.renderHeight = 1024;
    swapchainDescription.format = ResourceFormat_B8G8R8A8_Unorm;
    pSwapchain = pDevice->createSwapchain(swapchainDescription, pWindow->getNativeHandle());
    context = pContext;
    context->setFrames(2);
    F32 index = 200;
    F32 counterFps = 0.f;
    F32 desiredFps = 1.f / 60.f;
    Math::Float2 direction{ 1.f, -1.f };
    Math::Float2 boxSize = Math::Float2(200.0f, 200.0f);
    Math::Bounds2d bounds = { Math::Float2(0.f, 0.f), Math::Float2(1024.f, 1024.0f) };
    Math::Bounds2d box = { Math::Float2(412.f, 512.f), Math::Float2(0, 0) };
    box.mmax = boxSize + box.mmin;
    F32 speed = 200.f;
    F32 color[]         = { 0.0f, 1.0f, 0.0f, 1.0f };
    F32 color2[]        = { 0.0f, 0.0f, 0.2f, 1.0f };

    while (!pWindow->shouldClose()) 
    {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick   = RealtimeTick::getTick(0);
        F32 ms              = Limiter::limit(desiredFps, 1ull, 0);
        //R_TRACE("TEST", "FPS: %f", 1.0f / tick.getDeltaTimeS());
        box.mmin            = box.mmin + direction * speed * ms;
        box.mmax            = box.mmax + direction * speed * ms;
        
        while (!Math::contains(bounds, box))
        {
            Math::Float2 collisionN = { (box.mmin.x <= bounds.mmin.x) ? 1.f : (box.mmax.x > bounds.mmax.x ? -1.f : 0.f),
                                        (box.mmin.y <= bounds.mmin.y) ? 1.f : (box.mmax.y > bounds.mmax.y ? -1.f : 0.f) };
            
            collisionN  = Math::normalize(collisionN);
            direction   = Math::reflect(direction, collisionN);

            std::random_device rd;
            std::mt19937 twist(rd());
            std::uniform_real_distribution<F32> dist(-0.1f, 0.1f);
            std::uniform_real_distribution<F32> rgb(0.1f, 1.0f);

            direction = Math::normalize(direction + Math::Float2{ dist(twist), dist(twist) });

            color[0] = rgb(twist);
            color[1] = rgb(twist);
            color[2] = rgb(twist);

            box.mmin = box.mmin + direction * speed * ms;
            box.mmax = box.mmax + direction * speed * ms;
        }

        Rect rect           = { box.mmin.x, box.mmin.y, boxSize.x, boxSize.y };
        Rect rect2          = { bounds.mmin.x, bounds.mmin.y, bounds.mmax.x, bounds.mmax.y };

        pSwapchain->prepare(context);
            ResourceViewDescription rtvDesc = { };
            rtvDesc.type = ResourceViewType_RenderTarget;
            rtvDesc.baseArrayLayer = 0;
            rtvDesc.baseMipLevel = 0;
            rtvDesc.layerCount = 1;
            rtvDesc.mipLevelCount = 1;
            rtvDesc.dimension = ResourceViewDimension_2d;
            rtvDesc.format = ResourceFormat_B8G8R8A8_Unorm;
            ResourceViewId view = pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex())->asView(rtvDesc);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->bindRenderTargets(1, &view);
            context->clearRenderTarget(0, color2, rect2);
            context->clearRenderTarget(0, color, rect);
            context->bindRenderTargets(1, &view);
            context->clearRenderTarget(0, color2, rect2);
            context->clearRenderTarget(0, color, rect);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_Present);
        context->end();
        //R_FATAL_ERROR("TEST", "%f Fps", 1.0f / ms);
        pSwapchain->present(context);

        KeyboardListener listener;
        if (listener.isKeyDown(KeyCode_Escape))
        {
            pWindow->close();
        }
        
        pollEvents();
    }

    R_TRACE("TEST", "Exited game loop...");

    pContext->wait();    
    //pDevice->destroyResource(pData); 
    pDevice->destroySwapchain(pSwapchain);
    pDevice->releaseContext(context);
    pAdapter->destroyDevice(pDevice);
    Window::destroy(pWindow);
    GraphicsInstance::destroyInstance(pInstance);
    Log::destroyLoggingSystem();
    return 0;
}