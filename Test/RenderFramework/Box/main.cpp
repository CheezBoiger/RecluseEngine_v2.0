//
#include "Recluse/Types.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector4.hpp"

using namespace Recluse;


struct Vertex
{
    Math::Float3 position;
    Math::Float3 normal;    
};


std::vector<Vertex> boxCoordinates = 
{
     { { 0, 0, 0 }, {  0, 0, 0 } },
     { { }, { } }
};


std::vector<I16> boxIndicies =
{

};


enum VertexLayout
{
    VertexLayout_PositionNormal
};

enum ShaderProgram
{
    ShaderProgram_Box = 12354343
};

int main(char* argv[], int c)
{
    Log::initializeLoggingSystem();
    GraphicsInstance* instance  = GraphicsInstance::createInstance(GraphicsApi_Vulkan);
    GraphicsAdapter* adapter    = nullptr;
    GraphicsDevice* device      = nullptr;
    GraphicsContext* context    = nullptr;

    Window* window = Window::create("Box", 0, 0, 1024, 1024, ScreenMode_WindowBorderless);
    window->open();
    window->setToCenter();

    {
        ApplicationInfo appInfo = { };
        appInfo.engineName = "";
        appInfo.appName = "Box";
        appInfo.appMinor = 0;
        appInfo.appMajor = 0;
        appInfo.appPatch = 0;
        LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;
        R_ASSERT(instance->initialize(appInfo, flags) == RecluseResult_Ok);
    }
    
    adapter = instance->getGraphicsAdapters()[0];
    R_ASSERT(adapter);
    
    {
        DeviceCreateInfo devInfo = { };
        devInfo.winHandle                           = window->getNativeHandle();
        devInfo.swapchainDescription.buffering      = FrameBuffering_Double;
        devInfo.swapchainDescription.desiredFrames  = 3;
        devInfo.swapchainDescription.format         = ResourceFormat_B8G8R8A8_Unorm;
        devInfo.swapchainDescription.renderWidth    = window->getWidth();
        devInfo.swapchainDescription.renderHeight   = window->getHeight();
        R_ASSERT(adapter->createDevice(devInfo, &device) == RecluseResult_Ok);
    }

    context = device->createContext();
    R_ASSERT(context);
    context->setBuffers(2);
    
    while (!window->shouldClose())
    {
        context->begin();
            context->transition(device->getSwapchain()->getFrame(device->getSwapchain()->getCurrentFrameIndex()), ResourceState_RenderTarget);
            context->setShaderProgram(ShaderProgram_Box);
            context->setInputVertexLayout(VertexLayout_PositionNormal);
            context->enableDepth(true);
            context->setDepthCompareOp(CompareOp_GreaterOrEqual);
            context->bindIndexBuffer(nullptr, 0, IndexType_Unsigned16);
            context->drawIndexedInstanced(0, 0, 0, 0, 0);
            context->transition(device->getSwapchain()->getFrame(device->getSwapchain()->getCurrentFrameIndex()), ResourceState_Present);
        context->end();
        device->getSwapchain()->present();
        pollEvents();
    }
        
    context->wait();

    device->releaseContext(context);
    adapter->destroyDevice(device);
    GraphicsInstance::destroyInstance(instance);
    Log::destroyLoggingSystem();
    return 0;
}