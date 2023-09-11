
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
#include "Recluse/Graphics/ShaderProgram.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Time.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Archive.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"

#include "Recluse/System/Limiter.hpp"
#include "Recluse/System/KeyboardInput.hpp"

#include <cmath>

using namespace Recluse;


struct ConstData {
    float color[4];
    float pad[2];
    float iter[2];
    float resolution[4];
    float time;
    float pad0[3];
};


enum ProgramId
{
    ProgramId_Mandelbrot
};

GraphicsContext* pContext               = nullptr;
GraphicsDevice* pDevice                 = nullptr;
GraphicsSwapchain* pSwapchain           = nullptr;
GraphicsResource* output                = nullptr;


GraphicsResource* createUavResource(GraphicsResource*);

void ResizeFunction(U32 x, U32 y, U32 width, U32 height)
{
    if (!pSwapchain || !pDevice || !pContext) return;
    GraphicsSwapchain* swapchain = pSwapchain;
    SwapchainCreateDescription desc = swapchain->getDesc();
    //if (desc.renderWidth != width || desc.renderHeight != height)
    {
        if (width > 0 && height > 0)
        {
            desc.renderWidth = width;   
            desc.renderHeight = height;
            pContext->wait();
            swapchain->rebuild(desc);

            output = createUavResource(output);
        }
    }
}

void updateConstData(GraphicsResource* pData, U32 width, U32 height, F32 deltaSeconds, F32 currTimeSeconds)
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
    dat.resolution[0] = width;
    dat.resolution[1] = height;
    dat.time = currTimeSeconds;
    void* ptr = nullptr;
    pData->map(&ptr, nullptr);
    memcpy(ptr, &dat, sizeof(ConstData));
    pData->unmap(nullptr);
}


GraphicsResource* createUavResource(GraphicsResource* pPrevious)
{
    if (pPrevious)
    {
        pDevice->destroyResource(pPrevious);
    }
    GraphicsResource* output = nullptr;
    GraphicsResourceDescription desc = { };
    desc.dimension = ResourceDimension_2d;
    desc.width = pSwapchain->getDesc().renderWidth;
    desc.height = pSwapchain->getDesc().renderHeight;
    desc.depthOrArraySize = 1;
    desc.mipLevels = 1;
    desc.format = pSwapchain->getDesc().format;
    desc.samples = 1;
    desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
    desc.usage = ResourceUsage_CopySource | ResourceUsage_UnorderedAccess | ResourceUsage_ShaderResource;
    desc.name = "ComputeOutput";
        
    pDevice->createResource(&output, desc, ResourceState_UnorderedAccess);
    return output;
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Debug);
    RealtimeTick::initializeWatch(1ull, 0);
    enableLogTypes(LogType_Notify);
    GraphicsInstance* pInstance     = GraphicsInstance::createInstance(GraphicsApi_Vulkan);
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsResource* pData         = nullptr;
    PipelineState* pPipeline        = nullptr;
    Window* pWindow                 = Window::create("Compute", 0, 0, 1024, 1024, ScreenMode_Fullscreen);
    ResultCode result               = RecluseResult_Ok;

    pWindow->setOnWindowResize(ResizeFunction);
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

        LayerFeatureFlags flags = 0;//LayerFeatureFlag_DebugValidation | LayerFeatureFlag_Raytracing | LayerFeatureFlag_MeshShading /*| LayerFeatureFlag_DebugMarking*/;

        result = pInstance->initialize(app, flags);
    }
    
    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to initialize context!");   
    }

    pAdapter = pInstance->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info = { true };
        result = pAdapter->createDevice(info, &pDevice);
    }

    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription = { };
    swapchainDescription.buffering = FrameBuffering_Triple;
    swapchainDescription.desiredFrames = 3;
    swapchainDescription.renderWidth = pWindow->getWidth();
    swapchainDescription.renderHeight = pWindow->getHeight();
    swapchainDescription.format = ResourceFormat_R8G8B8A8_Unorm;
    pSwapchain = pDevice->createSwapchain(swapchainDescription, pWindow->getNativeHandle());

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
        desc.height = 1;
        desc.depthOrArraySize = 1;
        desc.mipLevels = 1;
        desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        desc.usage = ResourceUsage_ConstantBuffer;
        desc.name = "ComputeConstantBuffer";
        
        result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
    }

    output = createUavResource(nullptr);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create data resource!");
    }

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
        if (pInstance->getApi() == GraphicsApi_Direct3D12)
            GlobalCommands::setValue("ShaderBuilder.NameId", "dxc");
        ShaderProgramDatabase database = ShaderProgramDatabase("Compute.Database");
        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
        FileBufferData file;
        std::string shaderPath = currDir + "/" + "test.cs.hlsl";
        Pipeline::Builder::ShaderProgramDescription description = { };
        description.pipelineType = BindType_Compute;
        description.language = ShaderLang_Hlsl;
        description.compute.cs = shaderPath.c_str();
        description.compute.csName = "main";

        Pipeline::Builder::buildShaderProgramDefinitions(database, description, 0, pInstance->getApi() == GraphicsApi_Vulkan ? ShaderIntermediateCode_Spirv : ShaderIntermediateCode_Dxil);
        Runtime::buildShaderProgram(pDevice, database, ProgramId_Mandelbrot);
        database.clearShaderProgramDefinitions();
    }

    pWindow->show();
    pWindow->setToCenter();
    const F32 desiredFps = 240.0f;
    F32 desiredMs = 1.f / desiredFps;
    pContext = pDevice->createContext();
    pContext->setFrames(3);
    R_NOTIFY("Compute", "Window width=%d, height=%d", pWindow->getWidth(), pWindow->getHeight());
    F32 seconds = 0.f;
    while (!pWindow->shouldClose()) 
    {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        F32 frameMs = Limiter::limit(desiredMs, 1ull, 0);
        seconds += frameMs;
        if (!pWindow->isMinimized())
        {
            updateConstData(pData, pSwapchain->getDesc().renderWidth, pSwapchain->getDesc().renderHeight, frameMs, seconds);
            GraphicsContext* context = pContext;
            pSwapchain->prepare(context);
                ResourceViewDescription desc   = { };
                desc.type               = ResourceViewType_UnorderedAccess;
                desc.dimension          = ResourceViewDimension_2d;
                desc.format             = pSwapchain->getDesc().format;
                desc.layerCount         = 1;
                desc.mipLevelCount      = 1;
                desc.baseArrayLayer     = 0;
                desc.baseMipLevel       = 0;

                GraphicsResource* frame = pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex());
                ResourceViewId uavView = output->asView(desc);
                context->transition(output, ResourceState_UnorderedAccess);
                context->bindConstantBuffer(ShaderStage_Compute, 1, pData, 0, sizeof(ConstData));
                context->bindUnorderedAccessView(ShaderStage_Compute, 0, uavView);
                context->setShaderProgram(ProgramId_Mandelbrot);
                context->dispatch(Math::divUp(pWindow->getWidth(), 8u), Math::divUp(pWindow->getHeight(), 8u), 1);

                context->transition(frame, ResourceState_CopyDestination);
                context->transition(output, ResourceState_CopySource);
                context->copyResource(frame, output);

                context->transition(frame, ResourceState_Present);
            context->end();
            if (pSwapchain->present(context) == RecluseResult_NeedsUpdate)
            {
                pContext->wait();
                pSwapchain->rebuild(pSwapchain->getDesc());
            }
            R_VERBOSE("Test", "Frame: %f fps", 1.0f / frameMs);
        }

        KeyboardListener listener;
        if (listener.isKeyDown(KeyCode_Escape))
        {
            pWindow->close();
        }
        pollEvents();
    }
    
    pContext->wait();
    pDevice->destroySwapchain(pSwapchain);
    pDevice->destroyResource(output);
    pDevice->destroyResource(pData);
    pDevice->releaseContext(pContext);
    pAdapter->destroyDevice(pDevice);
    GraphicsInstance::destroyInstance(pInstance);
    Log::destroyLoggingSystem();
    return 0;
}