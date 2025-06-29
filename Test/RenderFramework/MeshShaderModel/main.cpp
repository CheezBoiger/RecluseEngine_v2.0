//
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/System/KeyboardInput.hpp"
#include "Recluse/System/Input.hpp"

using namespace Recluse;

GraphicsInstance* gInstance = nullptr;
GraphicsAdapter* gAdapter   = nullptr;
GraphicsDevice* gDevice     = nullptr;
GraphicsSwapchain* gSwapchain = nullptr;
GraphicsContext* gContext   = nullptr;

void resizeFunction(U32 x, U32 y, U32 width, U32 height)
{
    if (!gSwapchain || !gDevice || !gContext) return;
    SwapchainCreateDescription desc = gSwapchain->getDesc();
    if (width > 0 && height > 0)
    {
        desc.renderWidth = width;
        desc.renderHeight = height;
        gContext->wait();
        gSwapchain->rebuild(desc);

        // Any new swapchain rebuilds, need to also update the 
        // swapchain resources along with it.
        
    }
}

int main(int c, char* argv[])
{
    LogSystem::initializeLoggingSystem();
    LogSystem::enableLogTypes(LogType_Debug | LogType_Info);
    Window* window = Window::create("MeshShaderModel", 0, 0, 1920, 1080, ScreenMode_Windowed);
    window->setOnWindowResize(resizeFunction);
    R_ASSERT(window != NULL);

    gInstance = GraphicsInstance::create(GraphicsApi_Direct3D12);

    ApplicationInfo appInfo{};
    appInfo.engineName = "Test";
    appInfo.appName = "MeshShaderModel";
    LayerFeatureFlags flags = LayerFeatureFlag_MeshShading | LayerFeatureFlag_DebugMarking | LayerFeatureFlag_DebugValidation | LayerFeatureFlag_GpuDebugValidation;

    ResultCode result = gInstance->initialize(appInfo, flags);
    R_ASSERT(result == RecluseResult_Ok);

    {
        const std::vector<GraphicsAdapter*>& adapters = gInstance->getGraphicsAdapters();

        for (GraphicsAdapter* adapter : adapters)
        {
            AdapterInfo info{};
            result = adapter->getAdapterInfo(&info);

            if (result == RecluseResult_Ok)
            {
                if (info.type == AdapterInfo::Type_DiscreteGpu)
                {
                    gAdapter = adapter;
                    break;
                }
            }
        }
    }

    R_ASSERT(gAdapter);

    DeviceCreateInfo deviceCreateInfo{};
    gAdapter->createDevice(deviceCreateInfo, &gDevice);

    R_ASSERT(gDevice);

    SwapchainCreateDescription swapchainCreateInfo{};
    swapchainCreateInfo.buffering = FrameBuffering_Triple;
    swapchainCreateInfo.desiredFrames = 3;
    swapchainCreateInfo.format = ResourceFormat_R8G8B8A8_Unorm;
    swapchainCreateInfo.preferHDR = false;
    swapchainCreateInfo.renderWidth = window->getWidth();
    swapchainCreateInfo.renderHeight = window->getHeight();

    gSwapchain = gDevice->createSwapchain(swapchainCreateInfo, window->getNativeHandle());

    gContext = gDevice->createContext();

    gContext->setFrames(3);

    window->show();
    window->setToCenter();

    while (!window->shouldClose())
    {
        pollEvents();

        gSwapchain->prepare(gContext);
        
        GraphicsResource* frame = gSwapchain->getFrame(gSwapchain->getCurrentFrameIndex());
        gContext->transition(frame, ResourceState_Present);
        gContext->end();

        ResultCode swapchainResult = gSwapchain->present(gContext);

        if (swapchainResult == RecluseResult_NeedsUpdate)
        {
            gContext->wait();
            gSwapchain->rebuild(gSwapchain->getDesc());
        }
    }

    gContext->wait();

    gDevice->destroySwapchain(gSwapchain);
    gDevice->releaseContext(gContext);
    gAdapter->destroyDevice(gDevice);
    GraphicsInstance::destroyInstance(gInstance);
    LogSystem::destroyLoggingSystem();
    return 0;
}