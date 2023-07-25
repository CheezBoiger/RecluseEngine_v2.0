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
#include "Recluse/System/Mouse.hpp"

#include "Recluse/System/Limiter.hpp"

#include <cmath>

using namespace Recluse;

R_DECLARE_GLOBAL_BOOLEAN(g_globalVar, false, "Global.Var");
R_DECLARE_GLOBAL_F32(g_value, 1.0f, "Global.Floatie");

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

void ResizeFunction(U32 x, U32 y, U32 width, U32 height)
{
    GraphicsSwapchain* swapchain = pDevice->getSwapchain();
    SwapchainCreateDescription desc = swapchain->getDesc();
    if (desc.renderWidth != width || desc.renderHeight != height)
    {
        desc.renderWidth = width;   
        desc.renderHeight = height;
        pContext->wait();
        swapchain->rebuild(desc);
    }
}

void updateConstData(GraphicsResource* pData, RealtimeTick& tick, U64 offsetBytes)
{
    ConstData dat = { };
    dat.color[0] = abs(sinf(tick.getCurrentTimeS() * 0.0000001f));
    dat.color[1] = 0.0f;
    dat.color[2] = 1.0f;
    dat.color[3] = 0.0f;
    
    dat.offset[0] = 0.0f;
    dat.offset[1] = 0.0f;
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

    GraphicsInstance* pInstance             = GraphicsInstance::createInstance(GraphicsApi_Vulkan);
    GraphicsAdapter* pAdapter               = nullptr;
    GraphicsResource* pData                 = nullptr;

    GraphicsResource* pVertexBuffer         = nullptr;
    PipelineState* pPipeline                = nullptr;
    GraphicsSwapchain* pSwapchain           = nullptr;
    Window* pWindow                         = Window::create(u8"猫はいいですPipelineInitialization", 0, 0, 512, 512);
    Mouse* pMouse                           = new Mouse();
    ResultCode result                       = RecluseResult_Ok;

    pMouse->initialize("Mouse1");
    pWindow->setMouseHandle(pMouse);
    pWindow->setOnWindowResize(ResizeFunction);

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
        LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;
        result                  = pInstance->initialize(app, flags);
    }
    
    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to initialize context!");   
    }

    pAdapter = pInstance->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info                       = { };
        info.winHandle                              = pWindow->getNativeHandle();
        info.swapchainDescription                   = { };
        info.swapchainDescription.buffering         = FrameBuffering_Single;
        info.swapchainDescription.desiredFrames     = 3;
        info.swapchainDescription.format            = ResourceFormat_B8G8R8A8_Unorm;
        info.swapchainDescription.renderWidth       = pWindow->getWidth();
        info.swapchainDescription.renderHeight      = pWindow->getHeight();
        result                                      = pAdapter->createDevice(info, &pDevice);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("TEST", "Failed to create device!");
    }

    pContext = pDevice->createContext();
    pContext->setBuffers(3);

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
    
    pSwapchain = pDevice->getSwapchain();
   
    {
        GraphicsResourceDescription desc = { };
        desc.dimension = ResourceDimension_Buffer;
        desc.width = sizeof(ConstData) * pContext->obtainBufferCount();
        desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        desc.usage = ResourceUsage_ConstantBuffer;
        result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
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
        pDevice->createResource(&pVertexBuffer, desc, ResourceState_VertexBuffer);
    
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
        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
        std::string vsSource = currDir + "/" + "test.vs.glsl";
        std::string fsSource = currDir + "/" + "test.fs.glsl";

        Builder::ShaderProgramDescription description = { };
        description.pipelineType = BindType_Graphics;
        description.graphics.vs = vsSource.c_str();
        description.graphics.vsName = "main";
        
        description.graphics.ps = fsSource.c_str();
        description.graphics.psName = "main";

        description.graphics.ds = nullptr;
        description.graphics.gs = nullptr;
        description.graphics.hs = nullptr;

        Builder::buildShaderProgramDefinitions(description, ShaderKey_SimpleColor, ShaderIntermediateCode_Spirv);
        Builder::Runtime::buildShaderProgram(pDevice, 0);
        Builder::releaseShaderProgramDefinition(0);


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
        layout.vertexBindings[0].stride = 8; // 1 float == 4 bytes.

        layout.numVertexBindings = 1;

        Builder::Runtime::buildVertexInputLayout(pDevice, layout, VertexLayoutKey_PositionOnly);
    }
    
    pWindow->open();

    U64 offset = 0;

    GraphicsContext* context = pContext;
    while (!pWindow->shouldClose()) 
    {
        F32 ms = Limiter::limit(1.0f / 160.0f, 1ull, 0);
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        Viewport viewport = { };
        viewport.x = 0; viewport.y = 0;
        viewport.width = pWindow->getWidth(), viewport.height = pWindow->getHeight();
        viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

        Rect scissor = { };
        scissor.x = 0; scissor.y = 0;
        scissor.width = pWindow->getWidth(); scissor.height = pWindow->getHeight();
        context->begin();
            updateConstData(pData, tick, sizeof(ConstData) * pContext->obtainCurrentBufferIndex());
            context->pushState();
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->setCullMode(CullMode_None);
            context->setFrontFace(FrontFace_CounterClockwise);
            context->setPolygonMode(PolygonMode_Line);
            context->setTopology(PrimitiveTopology_TriangleList);
            context->setViewports(1, &viewport);
            context->setScissors(1, &scissor);
            context->setInputVertexLayout(VertexLayoutKey_PositionOnly);
            context->setShaderProgram(ShaderKey_SimpleColor);
            GraphicsResourceView* pView = pSwapchain->getFrameView(pSwapchain->getCurrentFrameIndex());
            context->setColorWriteMask(0, Color_Rgba);
            context->bindRenderTargets(1, &pView, nullptr);
            context->bindConstantBuffer(ShaderType_Vertex, 0, pData, sizeof(ConstData) * pContext->obtainCurrentBufferIndex(), sizeof(ConstData));
            context->bindVertexBuffers(1, &pVertexBuffer, &offset);
            context->drawInstanced(3, 1, 0, 0);
            context->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_Present);
            context->popState();
        context->end();
        R_VERBOSE("Game", "%f fps", 1.0f / ms);
        R_VERBOSE("Game", "g_globalVar=%d", g_globalVar);
        pSwapchain->present();

        pollEvents();
    
    }

    pContext->wait();

    pDevice->destroyResource(pVertexBuffer);
    pDevice->destroyResource(pData);
    pDevice->releaseContext(pContext);
    pAdapter->destroyDevice(pDevice);
    GraphicsInstance::destroyInstance(pInstance);
    Window::destroy(pWindow);
    pMouse->destroy();
    delete pMouse;
    Log::destroyLoggingSystem();
    return 0;
}