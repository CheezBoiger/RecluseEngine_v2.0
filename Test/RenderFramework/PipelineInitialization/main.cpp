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

#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Time.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Mouse.hpp"
#include "Recluse/System/KeyboardInput.hpp"

#include "Recluse/System/Limiter.hpp"

#include <cmath>

using namespace Recluse;

struct Vertex 
{
    F32 position[2];
};


Vertex vertices[] = 
{
    { -1.0f,  0.0f },
    {  0.0f, -1.0f },
    {  1.0f,  0.0f }
};


struct ConstData 
{
    float color[4];
    float pad[2];
    float offset[2];
};

enum VertexLayoutKey
{
    VertexLayoutKey_PositionOnly
};


enum ShaderKey
{
    ShaderKey_SimpleColor
};

GraphicsContext* pContext               = nullptr;
GraphicsDevice* pDevice                 = nullptr;
GraphicsSwapchain* pSwapchain           = nullptr;

void ResizeFunction(U32 x, U32 y, U32 width, U32 height)
{
    if (!pDevice) return;
    if (!pSwapchain) return;
    GraphicsSwapchain* swapchain = pSwapchain;
    SwapchainCreateDescription desc = swapchain->getDesc();
    if (desc.renderWidth != width || desc.renderHeight != height)
    {
        if (width > 0 && height > 0)
        {
            desc.renderWidth = width;   
            desc.renderHeight = height;
            pContext->wait();
            swapchain->rebuild(desc);
        }
    }
}

void updateConstData(GraphicsResource* pData, RealtimeTick& tick, U64 offsetBytes, const Math::Float2& posOffset)
{
    ConstData dat = { };
    static F32 t = 0.f;
    t += 1.0f * tick.delta();
    dat.color[0] = abs(sinf(t));
    dat.color[1] = 0.0f;
    dat.color[2] = 1.0f;
    dat.color[3] = 0.0f;
    
    dat.offset[0] = posOffset.x;
    dat.offset[1] = posOffset.y;
    void* ptr = nullptr;
    MapRange range = { };
    range.offsetBytes = offsetBytes;
    range.sizeBytes = sizeof(ConstData);
    pData->map(&ptr, &range);
    memcpy(ptr, &dat, sizeof(ConstData));
    pData->unmap(&range);
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    enableLogTypes(LogType_Debug);
    disableLogTypes(LogType_Warn);
    GraphicsInstance* pInstance             = GraphicsInstance::createInstance(GraphicsApi_Direct3D12);
    GraphicsAdapter* pAdapter               = nullptr;
    GraphicsResource* pData                 = nullptr;
    GraphicsResource* pData2                = nullptr;

    GraphicsResource* pVertexBuffer         = nullptr;
    PipelineState* pPipeline                = nullptr;
    Window* pWindow                         = Window::create(u8"猫はいいですPipelineInitialization", 0, 0, 512, 512);
    Mouse* pMouse                           = new Mouse();
    ResultCode result                       = RecluseResult_Ok;

    pMouse->initialize("Mouse1");
    pWindow->setMouseHandle(pMouse);
    pWindow->setOnWindowResize(ResizeFunction);
    pWindow->setToCenter();
    pWindow->show();
    std::vector<RenderPass*> passes;

    if (!pInstance) 
    { 
        R_ERROR("TEST", "Failed to create instance!");
        return -1;
    }

    {
        ApplicationInfo app     = { };
        app.engineName          = "Cat";
        app.appName             = "PipelineInitialization";
        LayerFeatureFlags flags = 0;// LayerFeatureFlag_DebugValidation | LayerFeatureFlag_GpuDebugValidation;
        result                  = pInstance->initialize(app, flags);
    }
    
    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to initialize context!");   
    }

    pAdapter = pInstance->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info                       = { };
        result                                      = pAdapter->createDevice(info, &pDevice);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create device!");
    }

    pContext = pDevice->createContext();
    pContext->setFrames(3);

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

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create swapchain!");
    }

    if (result != RecluseResult_Ok) 
    { 
        R_ERROR("TEST", "Failed to create descriptor set layout...");
    }
    
    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.buffering         = FrameBuffering_Triple;
    swapchainDescription.desiredFrames     = 3;
    swapchainDescription.format            = ResourceFormat_R8G8B8A8_Unorm;
    swapchainDescription.renderWidth       = pWindow->getWidth();
    swapchainDescription.renderHeight      = pWindow->getHeight();
    pSwapchain = pDevice->createSwapchain(swapchainDescription, pWindow->getNativeHandle());
   
    {
        GraphicsResourceDescription desc = { };
        desc.dimension = ResourceDimension_Buffer;
        desc.width              = pAdapter->constantBufferOffsetAlignmentBytes() * pContext->obtainFrameCount();
        desc.height             = 1;
        desc.depthOrArraySize   = 1;
        desc.mipLevels          = 1;
        desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        desc.usage = ResourceUsage_ConstantBuffer;
        result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
        result = pDevice->createResource(&pData2, desc, ResourceState_ConstantBuffer);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create data resource!");    
    } 
    else 
    {
        
        ConstData dat = { };
        dat.color[0] = 1.0f;
        dat.color[1] = 0.0f;
        dat.color[2] = 0.0f;
        dat.color[3] = 0.0f;
    
        dat.offset[0] = 0.0f;
        dat.offset[1] = 0.0f;
        
        void* ptr = nullptr;
        MapRange range = { };
        range.offsetBytes = 0;
        range.sizeBytes = sizeof(ConstData);
        pData->map(&ptr, &range);
        memcpy(ptr, &dat, sizeof(ConstData));
        pData->unmap(&range);
    }

    {
        GraphicsResource* pTemp = nullptr;
        GraphicsResourceDescription desc = { };
        desc.usage = ResourceUsage_VertexBuffer | ResourceUsage_TransferDestination;
        desc.width = sizeof(vertices);
        desc.dimension = ResourceDimension_Buffer;
        desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
        desc.depthOrArraySize = 1;
        desc.height = 1;
        desc.mipLevels = 1;
        pDevice->createResource(&pVertexBuffer, desc, ResourceState_CopyDestination);
    
        desc.memoryUsage = ResourceMemoryUsage_CpuOnly;
        desc.usage = ResourceUsage_TransferSource;
        pDevice->createResource(&pTemp, desc, ResourceState_CopySource);

        void* ptr = nullptr;
        MapRange range = { };

        range.sizeBytes = sizeof(vertices);
        range.offsetBytes = 0;
        
        pTemp->map(&ptr, &range);
        memcpy(ptr, vertices, sizeof(vertices));
        pTemp->unmap(&range);

        // result = pQueue->copyResource(pVertexBuffer, pTemp);
        CopyBufferRegion region = { };
        region.srcOffsetBytes = 0;
        region.dstOffsetBytes = 0;
        region.szBytes = sizeof(vertices);
        pDevice->copyBufferRegions(pVertexBuffer, pTemp, &region, 1);
        pDevice->destroyResource(pTemp);
    }

    {
        if (pInstance->getApi() == GraphicsApi_Direct3D12)
            GlobalCommands::setValue("ShaderBuilder.NameId", "dxc");
        ShaderProgramDatabase database          = ShaderProgramDatabase("PipelineInitialization.D3D12.Database");
        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
        std::string vsSource = currDir + "/" + "test.vs.hlsl";
        std::string fsSource = currDir + "/" + "test.fs.hlsl";

        Pipeline::Builder::ShaderProgramDescription description = { };
        description.pipelineType = BindType_Graphics;
        description.graphics.vs = vsSource.c_str();
        description.graphics.vsName = "main";
        
        description.graphics.ps = fsSource.c_str();
        description.graphics.psName = "main";

        description.graphics.ds = nullptr;
        description.graphics.gs = nullptr;
        description.graphics.hs = nullptr;

        description.language = ShaderLang_Hlsl;
        Pipeline::Builder::buildShaderProgramDefinitions(database, description, ShaderKey_SimpleColor, pInstance->getApi() == GraphicsApi_Direct3D12 ? ShaderIntermediateCode_Dxil : ShaderIntermediateCode_Spirv);
        Runtime::buildShaderProgram(pDevice, database, 0);
        database.clearShaderProgramDefinitions();

        VertexInputLayout layout    = { };
        VertexAttribute attrib      = { };
        attrib.format               = ResourceFormat_R32G32_Float;
        attrib.location             = 0;
        attrib.offsetBytes          = 0;
        attrib.semantic             = Semantic_Position;
        attrib.semanticIndex        = 0;

        layout.vertexBindings[0].binding = 0;
        layout.vertexBindings[0].inputRate = InputRate_PerVertex;
        layout.vertexBindings[0].numVertexAttributes = 1;
        layout.vertexBindings[0].pVertexAttributes = &attrib;
        //layout.vertexBindings[0].stride = 8; // 1 float == 4 bytes.

        layout.numVertexBindings = 1;

        Runtime::buildVertexInputLayout(pDevice, layout, VertexLayoutKey_PositionOnly);
    }

    U64 offset = 0;

    GraphicsContext* context = pContext;
    GlobalCommands::setValue("Vulkan.EnablePipelineCache", false);
    GlobalCommands::setValue("Vulkan.PipelineMaxAge", 144);
    while (!pWindow->shouldClose()) 
    {
        if (!pWindow->isMinimized())
        {
            F32 ms = Limiter::limit(0.0f / 160.0f, 1ull, 0);
            RealtimeTick::updateWatch(1ull, 0);
            RealtimeTick tick = RealtimeTick::getTick(0);
            Viewport viewport = { };
            viewport.x = 0; viewport.y = 0;
            viewport.width = pWindow->getWidth(), viewport.height = pWindow->getHeight();
            viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

            Rect scissor = { };
            scissor.x = 0; scissor.y = 0;
            scissor.width = pWindow->getWidth(); scissor.height = pWindow->getHeight();
            pSwapchain->prepare(pContext);
                updateConstData(pData, tick, pAdapter->constantBufferOffsetAlignmentBytes() * pContext->obtainCurrentFrameIndex(), Math::Float2(0, 0));
                updateConstData(pData2, tick, pAdapter->constantBufferOffsetAlignmentBytes() * pContext->obtainCurrentFrameIndex(), Math::Float2(0, 1));
                context->pushState();
                context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
                context->setCullMode(CullMode_None);
                context->setFrontFace(FrontFace_Clockwise);
                context->setPolygonMode(PolygonMode_Fill);
                context->setTopology(PrimitiveTopology_TriangleList);
                context->setViewports(1, &viewport);
                context->setScissors(1, &scissor);
                context->setInputVertexLayout(VertexLayoutKey_PositionOnly);
                context->setShaderProgram(ShaderKey_SimpleColor);
            
                ResourceViewDescription rtvDesc = { };
                rtvDesc.type                    = ResourceViewType_RenderTarget;
                rtvDesc.format                  = pSwapchain->getDesc().format;
                rtvDesc.dimension               = ResourceViewDimension_2d;
                rtvDesc.baseArrayLayer          = 0;
                rtvDesc.baseMipLevel            = 0;
                rtvDesc.layerCount              = 1;
                rtvDesc.mipLevelCount           = 1;
                ResourceViewId frameRtv = pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex())->asView(rtvDesc);
                context->setColorWriteMask(0, Color_Rgba);
                context->bindRenderTargets(1, &frameRtv);
                F32 clear[4] = { 0, 0, 0, 1 };
                Rect rect; rect.x = 0; rect.y = 0; rect.width = pSwapchain->getDesc().renderWidth; rect.height = pSwapchain->getDesc().renderHeight;
                context->clearRenderTarget(0, clear, rect);
                context->transition(pVertexBuffer, ResourceState_VertexBuffer);
                context->bindVertexBuffers(1, &pVertexBuffer, &offset);

                KeyboardListener listener = { };

                if (pMouse->getButtonState(1) == InputState_Down)
                {
                    context->bindConstantBuffer(ShaderStage_Fragment | ShaderStage_Vertex, 0, pData, pAdapter->constantBufferOffsetAlignmentBytes() * pContext->obtainCurrentFrameIndex(), sizeof(ConstData));                 
                    context->drawInstanced(3, 1, 0, 0);
                }
                if (pMouse->getButtonState(0) == InputState_Down)
                {
                    context->bindConstantBuffer(ShaderStage_Fragment | ShaderStage_Vertex, 0, pData2, pAdapter->constantBufferOffsetAlignmentBytes() * pContext->obtainCurrentFrameIndex(), sizeof(ConstData));
                    context->drawInstanced(3, 1, 0, 0);
                }

                context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_Present);
                context->popState();
            context->end();
            R_WARN("Game", "%f fps", 1.0f / ms);
            pSwapchain->present(pContext);
        }
        pollEvents();
    }

    pContext->wait();

    pDevice->destroySwapchain(pSwapchain);
    pDevice->destroyResource(pVertexBuffer);
    pDevice->destroyResource(pData);
    pDevice->destroyResource(pData2);
    pDevice->releaseContext(pContext);
    pAdapter->destroyDevice(pDevice);
    GraphicsInstance::destroyInstance(pInstance);
    Window::destroy(pWindow);
    pMouse->destroy();
    delete pMouse;
    Log::destroyLoggingSystem();
    return 0;
}