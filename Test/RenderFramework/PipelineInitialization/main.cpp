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

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Time.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/System/Mouse.hpp"

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

void updateConstData(GraphicsResource* pData, RealtimeTick& tick)
{
    ConstData dat = { };
    dat.color[0] = abs(sinf(tick.getCurrentTimeS() * 0.0000001f));
    dat.color[1] = 0.0f;
    dat.color[2] = 1.0f;
    dat.color[3] = 0.0f;
    
    dat.offset[0] = 0.0f;
    dat.offset[1] = 0.0f;
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
    GraphicsResource* pVertexBuffer = nullptr;
    GraphicsResourceView* pView     = nullptr;
    DescriptorSetLayout* pLayout    = nullptr;
    DescriptorSet* pSet             = nullptr;
    PipelineState* pPipeline        = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsCommandList* pList      = nullptr;
    Window* pWindow                 = Window::create("PipelineInitialization", 0, 0, 512, 512);
    Mouse* pMouse                   = new Mouse();
    ErrType result                  = RecluseResult_Ok;

    pMouse->initialize("Mouse1");
    pWindow->setMouseHandle(pMouse);

    std::vector<RenderPass*> passes;

    if (!pInstance) 
    { 
        R_ERR("TEST", "Failed to create instance!");
        return -1;
    }

    {
        ApplicationInfo app = { };
        app.engineName = "Cat";
        app.appName = "PipelineInitialization";

        EnableLayerFlags flags = LayerFeature_DebugValidationBit;

        result = pInstance->initialize(app, flags);
    }
    
    if (result != RecluseResult_Ok) 
    {
        R_ERR("TEST", "Failed to initialize context!");   
    }

    pAdapter = pInstance->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info = { };
        info.buffering = 3;
        info.winHandle = pWindow->getNativeHandle();
        info.swapchainDescription = { };
        info.swapchainDescription.buffering = FrameBuffering_Double;
        info.swapchainDescription.desiredFrames = 3;
        info.swapchainDescription.renderWidth = pWindow->getWidth();
        info.swapchainDescription.renderHeight = pWindow->getHeight();

        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR("TEST", "Failed to create device!");
    }

    {
        MemoryReserveDesc desc = { };
        desc.bufferPools[ResourceMemoryUsage_CpuOnly] = 1 * R_1MB;
        desc.bufferPools[ResourceMemoryUsage_CpuToGpu] = 1 * R_1MB;
        desc.bufferPools[ResourceMemoryUsage_GpuOnly] = 1 * R_1MB;
        desc.bufferPools[ResourceMemoryUsage_GpuToCpu] = 1 * R_1MB;
        desc.texturePoolGPUOnly = 1 * R_1MB;

        result = pDevice->reserveMemory(desc);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR("TEST", "Failed to reserve memory on device!");
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR("TEST", "Failed to create swapchain!");
    }

    {
        DescriptorSetLayoutDesc desc = { };
        desc.numDescriptorBinds = 1;

        DescriptorBindDesc bind = { };
        bind.binding = 0;
        bind.bindType = DescriptorBindType_ConstantBuffer;
        bind.numDescriptors = 1;
        bind.shaderStages = ShaderType_Vertex | ShaderType_Fragment;

        desc.pDescriptorBinds = &bind;
        
        result = pDevice->createDescriptorSetLayout(&pLayout, desc);
    }

    if (result != RecluseResult_Ok) 
    { 
        R_ERR("TEST", "Failed to create descriptor set layout...");
    }
    
    pSwapchain = pDevice->getSwapchain();
    
    {
        RenderPassDesc desc = { };
        desc.width = pWindow->getWidth();
        desc.height = pWindow->getHeight();
        desc.numRenderTargets = 1;
        desc.pDepthStencil = nullptr;

        passes.resize(pSwapchain->getDesc().desiredFrames);

        for (U32 i = 0; i < passes.size(); ++i) 
        {
            GraphicsResourceView* pView = pSwapchain->getFrameView(i);
            desc.ppRenderTargetViews[0] = pView;
            result = pDevice->createRenderPass(&passes[i], desc);      
            if (result != RecluseResult_Ok) 
            {
                R_ERR("TEST", "Failed to create render pass...");
            }
        }
    }

    result = pDevice->createDescriptorSet(&pSet, pLayout); 

    if (result != RecluseResult_Ok) 
    {
        R_ERR("TEST", "Failed to create descriptor set!");
    }

    {
        GraphicsResourceDescription desc = { };
        desc.dimension = ResourceDimension_Buffer;
        desc.width = sizeof(ConstData);
        desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        desc.usage = ResourceUsage_ConstantBuffer;
        
        result = pDevice->createResource(&pData, desc, ResourceState_ConstantBuffer);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERR("TEST", "Failed to create data resource!");    
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

        DescriptorSetBind bind = { };
        bind.binding = 0;
        bind.bindType = DescriptorBindType_ConstantBuffer;
        bind.descriptorCount = 1;
        bind.cb.buffer = pData;
        bind.cb.offset = 0;
        bind.cb.sizeBytes = sizeof(ConstData);    

        pSet->update(&bind, 1);
    }

    {
        GraphicsResource* pTemp = nullptr;
        GraphicsResourceDescription desc = { };
        desc.usage = ResourceUsage_VertexBuffer | ResourceUsage_TransferDestination;
        desc.width = sizeof(vertices);
        desc.dimension = ResourceDimension_Buffer;
        desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
        desc.depth = 1;
        pDevice->createResource(&pVertexBuffer, desc, ResourceState_VertexAndConstantBuffer);
    
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
        result = pDevice->getContext()->copyBufferRegions(pVertexBuffer, pTemp, &region, 1);

        if (result != RecluseResult_Ok) 
        {
            R_ERR("TEST", "Failed to stream vertex data to vertex buffer!");
        }    

        pDevice->destroyResource(pTemp);
    }

    {
        ShaderBuilder* pBuilder = createGlslangShaderBuilder(ShaderIntermediateCode_Spirv);
        pBuilder->setUp();

        Shader* pVertShader = Shader::create();
        Shader* pFragShader = Shader::create();

        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);

        FileBufferData file;
        std::string shaderSource = currDir + "/" + "test.vs.glsl";
        result = File::readFrom(&file, shaderSource);

        if (result != RecluseResult_Ok) 
        {
            R_ERR("TEST", "Could not find %s", shaderSource.c_str());
        }

        result = pBuilder->compile(pVertShader, file.data(), file.size(), ShaderLang_Glsl, ShaderType_Vertex);

        if (result != RecluseResult_Ok) 
        {
            R_ERR("TEST", "Failed to compile glsl shader vert!");
        }

        shaderSource = currDir + "/" + "test.fs.glsl";
        result = File::readFrom(&file, shaderSource);
            
        if (result != RecluseResult_Ok) 
        {
            R_ERR("TEST", "Could not find %s", shaderSource.c_str());
        }

        result = pBuilder->compile(pFragShader, file.data(), file.size(), ShaderLang_Glsl, ShaderType_Fragment);
        
        if (result != RecluseResult_Ok) 
        {
            R_ERR("TEST", "Failed to compile glsl shader frag!");
        }
        
        GraphicsPipelineStateDesc desc = { };
        desc.pVS = pVertShader;
        desc.pPS = pFragShader;
        
        VertexAttribute attrib = { };
        attrib.format = ResourceFormat_R32G32_Float;
        attrib.loc = 0;
        attrib.offset = 0;
        attrib.semantic = "POSITION";

        VertexBinding binding = { };
        binding.binding = 0;
        binding.inputRate = InputRate_PerVertex;
        binding.numVertexAttributes = 1;
        binding.pVertexAttributes = &attrib;
        binding.stride = 8; // 1 float == 4 bytes.

        RenderTargetBlendState blendTarget = { };
        blendTarget.blendEnable = false;
        blendTarget.colorWriteMask = Color_Rgba;

        desc.raster.cullMode = CullMode_None;
        desc.raster.frontFace = FrontFace_CounterClockwise;
        desc.raster.polygonMode = PolygonMode_Fill;
        desc.raster.depthClampEnable = false;
        desc.raster.lineWidth  = 1.0f;
        desc.primitiveTopology = PrimitiveTopology_TriangleList;
        desc.vi.numVertexBindings = 1;
        desc.vi.pVertexBindings = &binding;
        desc.numDescriptorSetLayouts = 1;
        desc.ppDescriptorLayouts = &pLayout;
        desc.blend.logicOp = LogicOp_NoOp;
        desc.blend.logicOpEnable = false;
        desc.blend.numAttachments = 1;
        desc.blend.attachments = &blendTarget;
        desc.pRenderPass = passes[0];

        result = pDevice->createGraphicsPipelineState(&pPipeline, desc);

        Shader::destroy(pVertShader);
        Shader::destroy(pFragShader);
        pBuilder->tearDown();
        freeShaderBuilder(pBuilder);
    }
    
    pWindow->open();

    Viewport viewport = { };
    viewport.x = 0; viewport.y = 0;
    viewport.width = 512, viewport.height = 512;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    Rect scissor = { };
    scissor.x = 0; scissor.y = 0;
    scissor.width = 512; scissor.height = 512;

    U64 offset = 0;

    while (!pWindow->shouldClose()) 
    {
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick = RealtimeTick::getTick(0);
        R_VERBOSE("Game", "%f", tick.delta());
        updateConstData(pData, tick);
        pDevice->getContext()->begin();    
        pList = pDevice->getContext()->getCommandList();
        pList->begin();
            pList->transition(pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex()), ResourceState_RenderTarget);
            pList->setRenderPass(passes[pSwapchain->getCurrentFrameIndex()]);
            pList->setPipelineState(pPipeline, BindType_Graphics);
            pList->setViewports(1, &viewport);
            pList->setScissors(1, &scissor);
            pList->bindDescriptorSets(1, &pSet, BindType_Graphics);
            pList->bindVertexBuffers(1, &pVertexBuffer, &offset);
            pList->drawInstanced(3, 1, 0, 0);
        pList->end();
        pDevice->getContext()->end();
        pSwapchain->present();

        pollEvents();
    
    }

    pDevice->getContext()->wait();

    for (U32 i = 0; i < passes.size(); ++i)
        pDevice->destroyRenderPass(passes[i]);

    pDevice->destroyResource(pVertexBuffer);
    pDevice->destroyResource(pData);
    pDevice->destroyDescriptorSet(pSet);
    pDevice->destroyPipelineState(pPipeline);
    pDevice->destroyDescriptorSetLayout(pLayout);
    pAdapter->destroyDevice(pDevice);
    GraphicsInstance::destroyInstance(pInstance);
    Window::destroy(pWindow);
    pMouse->destroy();
    delete pMouse;
    Log::destroyLoggingSystem();
    return 0;
}