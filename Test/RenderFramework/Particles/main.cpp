//
#include "Recluse/System/Window.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Logger.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Pipeline/Graphics/ShaderBuilder.hpp"
#include "Recluse/Pipeline/Graphics/ShaderPreprocessor.hpp"

#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/System/Input.hpp"
using namespace Recluse;

enum ProgramId
{
    ProgramId_ParticleComputeSimple = 1,
    ProgramId_ParticleRender
};

GraphicsResource* particlePositionsBuffer = nullptr;
GraphicsResource* particleVelocityBuffer = nullptr;

static void createShaderPrograms(GraphicsAPI api)
{   
    ShaderProgramDatabase database;
    Pipeline::ShaderBuilder* shaderBuilder = nullptr;
    ShaderIntermediateCode intermediateCode = ShaderIntermediateCode_Unknown;
    Pipeline::HlslToGlslPreprocessor preprocessor;
    preprocessor.setDebug(true);

    if (api == GraphicsApi_Direct3D12)
    {
        shaderBuilder = Pipeline::createShaderBuilder("dxc");
        intermediateCode = ShaderIntermediateCode_Dxil;
    }
    else
    {        
        shaderBuilder = Pipeline::createShaderBuilder("dxc");
        intermediateCode = ShaderIntermediateCode_Spirv;
        shaderBuilder->addPreprocessor(&preprocessor);   
    }

    shaderBuilder->setUp();
    
    // Descriptions are used for pipeline building.
    Pipeline::Builder::ShaderProgramDescription description;
    description.language = ShaderLanguage_Hlsl;

    FileBufferData source;

    std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
    std::string shaderFile = currDir + "/" + "particles.hlsl";

    File::readFrom(&source, shaderFile, File::NullTerminate);

    // Particle Compute.
    description.pipelineType = BindType_Compute;
    description.compute.cs = source.data();
    description.compute.csName = "ParticleComputeSimpleMain";
   
    // Build compute
    Pipeline::Builder::buildShaderProgram(database, description, ProgramId_ParticleComputeSimple, intermediateCode, shaderBuilder);

    // Particle Render
    description.pipelineType = BindType_Graphics;
    description.graphics.vs = source.data();
    description.graphics.vsName = "ParticleVertexMain";
    description.graphics.ps = source.data();
    description.graphics.psName = "ParticlePixelMain";

    // build render.
    Pipeline::Builder::buildShaderProgram(database, description, ProgramId_ParticleRender, intermediateCode, shaderBuilder);

    shaderBuilder->tearDown();
    Pipeline::freeShaderBuilder(shaderBuilder);
}

int main(int c, char* argv[])
{
    LogSystem::initializeLoggingSystem();
    LogSystem::enableLogTypes(LogType_Debug | LogType_Info);

    Window* window = Window::create("Particles", 0, 0, 1920, 1080, ScreenMode_Windowed);
    window->setToCenter();
    window->show();

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

    createShaderPrograms(instance->getApi());
    
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
    swapchainInfo.renderWidth = window->getWidth();
    swapchainInfo.renderHeight = window->getHeight();

    device->createSwapchain(swapchainInfo, window->getNativeHandle(), &swapchain);

    R_ASSERT(swapchain != NULL);

    GraphicsContext* context = device->createContext();

    context->setFrames(3);

    while (!window->shouldClose())
    {
        swapchain->prepare(context);
        GraphicsResource* swapchainFrame = swapchain->getFrame(swapchain->getCurrentFrameIndex());
        context->transition(swapchainFrame, ResourceState_Present);

        ShaderProgramBinder& binder = context->bindShaderProgram(ProgramId_ParticleComputeSimple, 0);
        binder.bindShaderResource(ShaderStage_All, 0, 0, ResourceView{0});
        context->dispatch(Math::divUp(64, 1), 1, 1);

        binder = context->bindShaderProgram(ProgramId_ParticleRender, 0);
        context->drawIndexedInstanced(3, 1, 0, 0, 0);

        context->end();
        swapchain->present();
        pollEvents();
    }
    context->wait();

    device->releaseContext(context);
    device->destroySwapchain(swapchain);
    adapter->destroyDevice(device);
    GraphicsInstance::destroyInstance(instance);
    Window::destroy(window);
    LogSystem::destroyLoggingSystem();
    return 0;
}