
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/CommandQueue.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/RenderPass.hpp"

#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"

using namespace Recluse;


struct ConstData {
    float color[4];
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    GraphicsContext* pContext       = GraphicsContext::createContext(GRAPHICS_API_VULKAN);
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsResource* pData         = nullptr;
    GraphicsResourceView* pView     = nullptr;
    DescriptorSetLayout* pLayout    = nullptr;
    GraphicsQueue* pQueue           = nullptr;
    DescriptorSet* pSet             = nullptr;
    PipelineState* pPipeline        = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsCommandList* pList      = nullptr;
    Window* pWindow                 = Window::create("PipelineInitialization", 0, 0, 512, 512);
    ErrType result                  = REC_RESULT_OK;

    std::vector<RenderPass*> passes;

    if (!pContext) { 
    
        R_ERR("TEST", "Failed to create context!");
    
        return -1;
    
    }

    {
        ApplicationInfo app = { };
        app.engineName = "Cat";
        app.appName = "PipelineInitialization";

        EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

        result = pContext->initialize(app, flags);
    }
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to initialize context!");
        
    }

    pAdapter = pContext->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info = { };
        info.buffering = 3;
        info.winHandle = pWindow->getNativeHandle();

        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create device!");
    
    }

    {
        MemoryReserveDesc desc = { };
        desc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY] = 1 * R_1MB;
        desc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_TO_GPU] = 1 * R_1MB;
        desc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY] = 1 * R_1MB;
        desc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_TO_CPU] = 1 * R_1MB;
        desc.texturePoolGPUOnly = 1 * R_1MB;

        result = pDevice->reserveMemory(desc);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to reserve memory on device!");
    
    }

    result = pDevice->createCommandQueue(&pQueue, QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_PRESENT);
    
    if (result != REC_RESULT_OK) { 
    
        R_ERR("TEST", "Failed to create command queue...");
    
    }

    {
        SwapchainCreateDescription info = { };
        info.buffering = FRAME_BUFFERING_DOUBLE;
        info.desiredFrames = 3;
        info.pBackbufferQueue = pQueue;
        info.renderWidth = pWindow->getWidth();
        info.renderHeight = pWindow->getHeight();
        result = pDevice->createSwapchain(&pSwapchain, info);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create swapchain!");    

    }

    {
        DescriptorSetLayoutDesc desc = { };
        desc.numDescriptorBinds = 1;

        DescriptorBindDesc bind = { };
        bind.binding = 0;
        bind.bindType = DESCRIPTOR_CONSTANT_BUFFER;
        bind.numDescriptors = 1;
        bind.shaderStages = SHADER_TYPE_VERTEX | SHADER_TYPE_FRAGEMENT;

        desc.pDescriptorBinds = &bind;
        
        result = pDevice->createDescriptorSetLayout(&pLayout, desc);
    }

    {
        Shader* pVertShader = Shader::create(INTERMEDIATE_SPIRV, SHADER_TYPE_VERTEX);
        Shader* pFragShader = Shader::create(INTERMEDIATE_SPIRV, SHADER_TYPE_PIXEL);
        
        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);

        File file;
        std::string shaderSource = currDir + "/" + "test.vs.glsl";
        result = File::readFrom(&file, shaderSource);

        if (result != REC_RESULT_OK) {
    
            R_ERR("TEST", "Could not find %s", shaderSource.c_str());
    
        }

        result = pVertShader->compile(file.data.data(), file.data.size());

        if (result != REC_RESULT_OK) {
    
            R_ERR("TEST", "Failed to compile glsl shader vert!");
    
        }

        shaderSource = currDir + "/" + "test.fs.glsl";
        result = File::readFrom(&file, shaderSource);
            
        if (result != REC_RESULT_OK) {
    
            R_ERR("TEST", "Could not find %s", shaderSource.c_str());
    
        }

        result = pFragShader->compile(file.data.data(), file.data.size());
        
        if (result != REC_RESULT_OK) {
    
            R_ERR("TEST", "Failed to compile glsl shader frag!");
    
        }
        
        GraphicsPipelineStateDesc desc = { };
        desc.pVS = pVertShader;
        desc.pPS = pFragShader;
        
        VertexAttribute attrib = { };
        attrib.format = RESOURCE_FORMAT_R32G32B32A32_FLOAT;
        attrib.loc = 0;
        attrib.offset = 0;
        attrib.semantic = "POSITION";

        VertexBinding binding = { };
        binding.binding = 0;
        binding.inputRate = INPUT_RATE_PER_VERTEX;
        binding.numVertexAttributes = 1;
        binding.pVertexAttributes = &attrib;
        binding.stride = 16; // 1 float == 4 bytes.

        desc.raster.cullMode = CULL_MODE_NONE;
        desc.raster.frontFace = FRONT_FACE_CLOCKWISE;
        desc.raster.polygonMode = POLYGON_MODE_FILL;
        desc.raster.depthClampEnable = false;
        desc.primitiveTopology = PRIMITIVE_TOPOLOGY_LINE_LIST;
        desc.vi.numVertexBindings = 1;
        desc.vi.pVertexBindings = &binding;
        desc.numDescriptorSetLayouts = 1;
        desc.ppDescriptorLayouts = &pLayout;

        Shader::destroy(pVertShader);
        Shader::destroy(pFragShader);
        
    }
    
    pWindow->open();

    while (!pWindow->shouldClose()) {
    

        pSwapchain->present();

        pollEvents();
    
    }
    
    pQueue->wait();

    pDevice->destroyDescriptorSetLayout(pLayout);
    pDevice->destroySwapchain(pSwapchain);
    pDevice->destroyCommandQueue(pQueue);
    pAdapter->destroyDevice(pDevice);
    GraphicsContext::destroyContext(pContext);
    Log::destroyLoggingSystem();
    return 0;
}