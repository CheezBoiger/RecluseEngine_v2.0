
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/ShaderBuilder.hpp"
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Time.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/System/Limiter.hpp"

#include <cmath>

using namespace Recluse;


struct ConstData {
    float color[4];
    float pad[2];
    float iter[2];
};


enum ProgramId
{
    ProgramId_Mandelbrot
};

void updateConstData(GraphicsResource* pData, F32 deltaSeconds, F32 currTimeSeconds)
{
    static float iter = 0.005f;
    iter += deltaSeconds;
    ConstData dat = { };
    dat.color[0] = abs(sinf(iter));
    dat.color[1] = 0.0f;
    dat.color[2] = 1.0f;
    dat.color[3] = 0.0f;
    
    dat.iter[0] = 0.0f;
    dat.iter[1] = iter;
    void* ptr = nullptr;
    pData->map(&ptr, nullptr);
    memcpy(ptr, &dat, sizeof(ConstData));
    pData->unmap(nullptr);
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);

    GraphicsInstance* pInstance       = GraphicsInstance::createInstance(GraphicsApi_Vulkan);
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsResource* pData         = nullptr;
    PipelineState* pPipeline        = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsContext* pContext       = nullptr;
    Window* pWindow                 = Window::create("Compute", 0, 0, 1024, 1024);
    ResultCode result                  = RecluseResult_Ok;

    std::vector<GraphicsResourceView*> views;

    if (!pInstance) 
    { 
        R_ERROR("TEST", "Failed to create context!");
        return -1;
    }

    {
        ApplicationInfo app = { };
        app.engineName = "Cat";
        app.appName = "Compute";

        LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation | LayerFeatureFlag_Raytracing | LayerFeatureFlag_MeshShading | LayerFeatureFlag_DebugMarking;

        result = pInstance->initialize(app, flags);
    }
    
    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to initialize context!");   
    }

    pAdapter = pInstance->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info = { };
        info.winHandle = pWindow->getNativeHandle();
        info.swapchainDescription = { };
        info.swapchainDescription.buffering = FrameBuffering_Triple;
        info.swapchainDescription.desiredFrames = 3;
        info.swapchainDescription.renderWidth = pWindow->getWidth();
        info.swapchainDescription.renderHeight = pWindow->getHeight();
        info.swapchainDescription.format = ResourceFormat_B8G8R8A8_Unorm;

        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create device!");
    }

    {
        MemoryReserveDescription desc = { };
        desc.bufferPools[ResourceMemoryUsage_CpuOnly] = 1 * R_1MB;
        desc.bufferPools[ResourceMemoryUsage_CpuToGpu] = 1 * R_1MB;
        desc.bufferPools[ResourceMemoryUsage_GpuOnly] = 1 * R_1MB;
        desc.bufferPools[ResourceMemoryUsage_GpuToCpu] = 1 * R_1MB;
        desc.texturePoolGPUOnly = 1 * R_1MB;

        result = pDevice->reserveMemory(desc);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to reserve memory on device!");
    }

    {
        GraphicsResourceDescription desc = { };
        desc.dimension = ResourceDimension_Buffer;
        desc.width = sizeof(ConstData);
        desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        desc.usage = ResourceUsage_ConstantBuffer;
        desc.name = "ComputeConstantBuffer";
        
        result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create data resource!");
    }

    pSwapchain = pDevice->getSwapchain();

    {
        ConstData dat = { };
        dat.color[0] = 1.0f;
        dat.color[1] = 0.0f;
        dat.color[2] = 0.0f;
        dat.color[3] = 0.0f;
    
        dat.iter[0] = 0.0f;
        dat.iter[1] = 0.0f;
        
        void* ptr = nullptr;
        MapRange range = { };
        range.offsetBytes = 0;
        range.sizeBytes = sizeof(ConstData);
        pData->map(&ptr, &range);
        memcpy(ptr, &dat, sizeof(ConstData));
        pData->unmap(&range);
    }

    {
        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
        FileBufferData file;
        std::string shaderPath = currDir + "/" + "test.cs.hlsl";
        Builder::ShaderProgramDescription description = { };
        description.pipelineType = BindType_Compute;
        description.language = ShaderLang_Hlsl;
        description.compute.cs = shaderPath.c_str();
        description.compute.csName = "main";
        Builder::buildShaderProgramDefinitions(description, 0, ShaderIntermediateCode_Spirv);
        Builder::Runtime::buildShaderProgram(pDevice, ProgramId_Mandelbrot);
        Builder::clearShaderProgramDefinitions();
    }

    pWindow->open();

    const F32 desiredFps = 144.0f;
    F32 desiredMs = 1.f / desiredFps;
    pContext = pDevice->createContext();
    pContext->setBuffers(3);

    while (!pWindow->shouldClose()) 
    {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        F32 frameMs = Limiter::limit(desiredMs, 1ull, 0);
        updateConstData(pData, frameMs, tick.getCurrentTimeS());
        GraphicsContext* context = pContext;
        context->begin();
            ResourceViewDescription desc   = { };
            desc.type               = ResourceViewType_UnorderedAccess;
            desc.dimension          = ResourceViewDimension_2d;
            desc.format             = ResourceFormat_B8G8R8A8_Unorm;
            desc.layerCount         = 1;
            desc.mipLevelCount      = 1;
            desc.baseArrayLayer     = 0;
            desc.baseMipLevel       = 0;

            GraphicsResource* frame = pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex());
            ResourceViewId uavView = frame->asView(desc);
            context->transition(frame, ResourceState_UnorderedAccess);
            context->bindConstantBuffer(ShaderType_Compute, 0, pData, 0, sizeof(ConstData));
            context->bindUnorderedAccessView(ShaderType_Compute, 0, uavView);
            context->setShaderProgram(ProgramId_Mandelbrot);
            context->dispatch(Math::divUp(pWindow->getWidth(), 8u), Math::divUp(pWindow->getHeight(), 8u), 1);
            context->transition(frame, ResourceState_Present);
        context->end();
        R_VERBOSE("Test", "Frame: %f fps", 1.0f / frameMs);
        pSwapchain->present();

        pollEvents();
    }
    
    pContext->wait();
    pDevice->destroyResource(pData);
    pDevice->releaseContext(pContext);
    pAdapter->destroyDevice(pDevice);
    GraphicsInstance::destroyInstance(pInstance);
    Log::destroyLoggingSystem();
    return 0;
}