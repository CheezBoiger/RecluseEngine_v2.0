//
#include "Recluse/System/Window.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Recluse/Pipeline/Graphics/ShaderPreprocessor.hpp"

using namespace Recluse;

enum ProgramId
{
    ProgramId_ParticleCompute = 1,
    ProgramId_ParticleRender
};

static void createShaderPrograms(GraphicsDevice* graphicsDevice)
{
    R_ASSERT(graphicsDevice);
    
    ShaderProgramDatabase database;
    
    // Descriptions are used for pipeline building.
    Pipeline::Builder::ShaderProgramDescription description;

    // Particle Compute.
    description.pipelineType = BindType_Compute;
    

    // Particle Render
    description.pipelineType = BindType_Graphics;
}

int main(int c, char* argv[])
{
    GraphicsInstance* instance = GraphicsInstance::create(GraphicsApi_Direct3D12);
    ResultCode result = RecluseResult_Ok;

    R_ASSERT(instance != NULL);
    ApplicationInfo appInfo = { };
    appInfo.appName = "Particles";
    appInfo.engineName = "Engine";

    result = instance->initialize(appInfo, 0);
    const std::vector<GraphicsAdapter*>& adapters = instance->getGraphicsAdapters();

    GraphicsAdapter* adapter = nullptr;

    for (uint i = 0; i < adapters.size(); ++i)
    {
        AdapterInfo info = { };
        GraphicsAdapter* tempAdapter = adapters[i];
        result = tempAdapter->getAdapterInfo(&info);
        if (info.type == AdapterInfo::Type_DiscreteGpu) 
        {
            adapter = tempAdapter;
            break;
        } 
        else if (info.type == AdapterInfo::Type_IntegratedGpu) 
        {
            adapter = tempAdapter;
        }
    }
    
    R_ASSERT(adapter != NULL);
    
    DeviceCreateInfo createInfo         = { };
    createInfo.allowAsyncCompute        = false;
    createInfo.enableDescriptorCaching  = false;

    GraphicsDevice* device = nullptr;
    
    result = adapter->createDevice(createInfo, &device);

    GraphicsSwapchain* swapchain = nullptr;
    SwapchainCreateDescription swapchainInfo = { };
    swapchainInfo.buffering = FrameBuffering_Double;
    swapchainInfo.desiredFrames = 3;
    swapchainInfo.format = ResourceFormat_R8G8B8A8_Unorm;
    swapchainInfo.preferHDR = false;
    swapchainInfo.renderWidth = 1920;
    swapchainInfo.renderHeight = 1080;

    device->createSwapchain(swapchainInfo, nullptr, &swapchain);

    R_ASSERT(swapchain != NULL);

    GraphicsContext* context = device->createContext();

    context->setFrames(3);

    swapchain->prepare(context);
        

    context->end();
    
    context->wait();
    swapchain->present(context);

    device->releaseContext(context);
    device->destroySwapchain(swapchain);
    adapter->destroyDevice(device);
    GraphicsInstance::destroyInstance(instance);
    return 0;
}