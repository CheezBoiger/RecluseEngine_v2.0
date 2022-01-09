
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

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Messaging.hpp"

#include <cmath>

using namespace Recluse;


struct ConstData {
    float color[4];
    float pad[2];
    float iter[2];
};

void updateConstData(GraphicsResource* pData, RealtimeTick& tick)
{
    static float iter = 0.005f;
    iter += 0.001f * tick.getDeltaTimeS();
    ConstData dat = { };
    dat.color[0] = abs(sinf(tick.getCurrentTimeS() * 0.0000001f));
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
    RealtimeTick::initialize();

    GraphicsInstance* pInstance       = GraphicsInstance::createInstance(GRAPHICS_API_VULKAN);
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsResource* pData         = nullptr;
    DescriptorSetLayout* pLayout    = nullptr;
    PipelineState* pPipeline        = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsCommandList* pList      = nullptr;
    Window* pWindow                 = Window::create("Compute", 0, 0, 1024, 1024);
    ErrType result                  = REC_RESULT_OK;

    std::vector<GraphicsResourceView*> views;
    std::vector<DescriptorSet*> sets;

    if (!pInstance) 
    { 
        R_ERR("TEST", "Failed to create context!");
        return -1;
    }

    {
        ApplicationInfo app = { };
        app.engineName = "Cat";
        app.appName = "Compute";

        EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

        result = pInstance->initialize(app, flags);
    }
    
    if (result != REC_RESULT_OK) 
    {
        R_ERR("TEST", "Failed to initialize context!");   
    }

    pAdapter = pInstance->getGraphicsAdapters()[0];

    {
        DeviceCreateInfo info = { };
        info.buffering = 3;
        info.winHandle = pWindow->getNativeHandle();
        info.swapchainDescription = { };
        info.swapchainDescription.buffering = FRAME_BUFFERING_DOUBLE;
        info.swapchainDescription.desiredFrames = 3;
        info.swapchainDescription.renderWidth = pWindow->getWidth();
        info.swapchainDescription.renderHeight = pWindow->getHeight();

        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != REC_RESULT_OK) 
    {
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

    if (result != REC_RESULT_OK) 
    {
        R_ERR("TEST", "Failed to reserve memory on device!");
    }

    {
        DescriptorSetLayoutDesc desc = { };
        desc.numDescriptorBinds = 2;

        DescriptorBindDesc bind[2] = { };
        bind[0].binding = 0;
        bind[0].bindType = DESCRIPTOR_CONSTANT_BUFFER;
        bind[0].numDescriptors = 1;
        bind[0].shaderStages = SHADER_TYPE_COMPUTE;
    
        bind[1].binding = 1;
        bind[1].bindType = DESCRIPTOR_STORAGE_IMAGE;
        bind[1].numDescriptors = 1;
        bind[1].shaderStages = SHADER_TYPE_COMPUTE;

        desc.pDescriptorBinds = bind;
        
        result = pDevice->createDescriptorSetLayout(&pLayout, desc);
    }

    if (result != REC_RESULT_OK) 
    {
        R_ERR("TEST", "Failed to create descriptor set layout...");
    }
    
    
    pList = pDevice->getCommandList();
    pSwapchain = pDevice->getSwapchain();

    sets.resize(pSwapchain->getDesc().desiredFrames);
    for (U32 i = 0; i < sets.size(); ++i) 
    {
        result = pDevice->createDescriptorSet(&sets[i], pLayout); 
        if (result != REC_RESULT_OK) 
        {
            R_ERR("TEST", "Failed to create descriptor set!");
        }
    }
    

    {
        GraphicsResourceDescription desc = { };
        desc.dimension = RESOURCE_DIMENSION_BUFFER;
        desc.width = sizeof(ConstData);
        desc.memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        desc.usage = RESOURCE_USAGE_CONSTANT_BUFFER;
        
        result = pDevice->createResource(&pData, desc, RESOURCE_STATE_VERTEX_AND_CONST_BUFFER);
    }

    if (result != REC_RESULT_OK) 
    {
        R_ERR("TEST", "Failed to create data resource!");
    }

    {
        ResourceViewDesc desc   = { };
        desc.type               = RESOURCE_VIEW_TYPE_STORAGE_IMAGE;
        desc.dimension          = RESOURCE_VIEW_DIMENSION_2D;
        desc.format             = RESOURCE_FORMAT_R8G8B8A8_UNORM;
        desc.layerCount         = 1;
        desc.mipLevelCount      = 1;
        desc.baseArrayLayer     = 0;
        desc.baseMipLevel       = 0;

        views.resize(pSwapchain->getDesc().desiredFrames);
        for (U32 i = 0; i < pSwapchain->getDesc().desiredFrames; ++i) 
        {
            desc.pResource = pSwapchain->getFrame(i);
            pDevice->createResourceView(&views[i], desc);
        }
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

        for (U32 i = 0; i < sets.size(); ++i) 
        {
            DescriptorSetBind bind[2] = { };
            bind[0].binding = 0;
            bind[0].bindType = DESCRIPTOR_CONSTANT_BUFFER;
            bind[0].descriptorCount = 1;
            bind[0].cb.buffer = pData;
            bind[0].cb.offset = 0;
            bind[0].cb.sizeBytes = sizeof(ConstData);

            bind[1].binding = 1;
            bind[1].bindType = DESCRIPTOR_STORAGE_IMAGE;
            bind[1].descriptorCount = 1;
            bind[1].srv.pView = views[i];

            sets[i]->update(bind, 2);
        }
    }

    {
        ShaderBuilder* pBuilder = createGlslangShaderBuilder(INTERMEDIATE_SPIRV);
        pBuilder->setUp();
        Shader* pCompShader = Shader::create();
        std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
        FileBufferData file;
        std::string shaderSource = currDir + "/" + "test.cs.hlsl";
        result = File::readFrom(&file, shaderSource);

        if (result != REC_RESULT_OK) 
        {    
            R_ERR("TEST", "Could not find %s", shaderSource.c_str());
        }

        result = pBuilder->compile(pCompShader, file.data(), file.size(), SHADER_LANG_HLSL, SHADER_TYPE_COMPUTE);

        if (result != REC_RESULT_OK) 
        {
            R_ERR("TEST", "Failed to compile compute shader!");
        }

        ComputePipelineStateDesc ci = { };
        ci.numDescriptorSetLayouts = 1;
        ci.ppDescriptorLayouts = &pLayout;
        ci.pCS = pCompShader;
        ci.pRenderPass = nullptr;

        result = pDevice->createComputePipelineState(&pPipeline, ci);

        if (result != REC_RESULT_OK) 
        {
            R_ERR("TEST", "Failed to create compute pipeline!");
        }

        Shader::destroy(pCompShader);
        pBuilder->tearDown();
        freeShaderBuilder(pBuilder);
    }

    pWindow->open();

    while (!pWindow->shouldClose()) 
    {
        RealtimeTick tick = RealtimeTick::getTick();
        updateConstData(pData, tick);

        pList->begin();
            GraphicsResource* pResource = views[pSwapchain->getCurrentFrameIndex()]->getResource();
            ResourceTransition tran = MAKE_RESOURCE_TRANSITION(pResource, RESOURCE_STATE_STORAGE, 0, 1, 0, 1);
            pList->transition(&tran, 1);
            pList->setPipelineState(pPipeline, BIND_TYPE_COMPUTE);
            pList->bindDescriptorSets(1, &sets[pSwapchain->getCurrentFrameIndex()], BIND_TYPE_COMPUTE);
            pList->dispatch(pWindow->getWidth() / 8 + 1, pWindow->getHeight() / 8 + 1, 1);
        pList->end();

        pSwapchain->present();

        pollEvents();
    
    }
    
    pDevice->wait();

    pDevice->destroyResource(pData);

    for (U32 i = 0; i < sets.size(); ++i)
        pDevice->destroyDescriptorSet(sets[i]);

    for (U32 i = 0; i < views.size(); ++i)
        pDevice->destroyResourceView(views[i]);

    pDevice->destroyPipelineState(pPipeline);
    pDevice->destroyDescriptorSetLayout(pLayout);
    pAdapter->destroyDevice(pDevice);
    GraphicsInstance::destroyInstance(pInstance);
    Log::destroyLoggingSystem();
    return 0;
}


