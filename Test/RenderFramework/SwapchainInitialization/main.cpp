#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/CommandList.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/System/KeyboardInput.hpp"

using namespace Recluse;


GraphicsSwapchain* pSwapchain   = nullptr;
GraphicsDevice* pDevice         = nullptr;
GraphicsContext* pContext       = nullptr;

void resizeFunc(U32 x, U32 y, U32 width, U32 height)
{
    if (!pSwapchain || !pDevice || !pContext) return;
    if (width > 0 && height > 0)
    {
        SwapchainCreateDescription description = pSwapchain->getDesc();
        description.renderWidth = width;
        description.renderHeight = height;
        pContext->wait();
        pSwapchain->rebuild(description);
    }
}

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    GraphicsInstance* pInstance       = GraphicsInstance::create(GraphicsApi_Vulkan);

    Window* pWindow = Window::create(u8"SwapchainInitialization", 0, 0, 1280, 720, ScreenMode_Windowed);
    //Window* pWindow2 = Window::create(u8"Window2", 0, 0, 800, 800, ScreenMode_Windowed);

    if (!pInstance || !pWindow) {
        goto Exit;
    }
    pWindow->setOnWindowResize(resizeFunc);
    ApplicationInfo appInfo = { };

    appInfo.appMajor    = 0;
    appInfo.appMinor    = 0;
    appInfo.appName     = "SwapchainInitialization";
    appInfo.appPatch    = 0;
    appInfo.engineMajor = 0;
    appInfo.engineMinor = 0;
    appInfo.engineName  = "None";
    appInfo.enginePatch = 0;

    LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;

    ResultCode result = pInstance->initialize(appInfo, flags);

    if (result != RecluseResult_Ok) {
        R_ERROR("Test", "Failed to create context!");
        goto Exit;
    }

    std::vector<GraphicsAdapter*>& adapters = pInstance->getGraphicsAdapters();

    for (GraphicsAdapter* adapter : adapters) {

        AdapterInfo adapterInfo = { }; 
        adapter->getAdapterInfo(&adapterInfo);

        R_INFO("Graphics", "\tDevice Name: %s\n\t\tVendor ID: %d\n", adapterInfo.deviceName, adapterInfo.vendorId);

    }

    DeviceCreateInfo deviceCreate   = { true };

    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.buffering                    = FrameBuffering_Double;
    swapchainDescription.desiredFrames                = 3;
    swapchainDescription.renderHeight                 = 720;
    swapchainDescription.renderWidth                  = 1280;
    swapchainDescription.format                       = ResourceFormat_B8G8R8A8_Unorm;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != RecluseResult_Ok) {
    
        R_ERROR("Graphics", "Failed to create device!");

    }
    pContext = pDevice->createContext();
    pContext->setFrames(3);
    pWindow->setToCenter();

    pSwapchain = pDevice->createSwapchain(swapchainDescription, pWindow->getNativeHandle());
    
    if (!pSwapchain) {
    
        R_ERROR("Graphics", "Failed to create swapchain in test!");
    
    } else {
    
        R_TRACE("Graphics", "Succeeded swapchain creation!");

        pWindow->show();
        //pWindow2->open();
        while (!pWindow->shouldClose()) 
        {
            if (!pWindow->isMinimized())
            {
                RealtimeTick::updateWatch(1ull, 0);
                RealtimeTick tick = RealtimeTick::getTick(0);
                R_WARN("Graphics", "FPS: %f", 1.f / tick.delta());
                pSwapchain->prepare(pContext);
                    ResourceViewDescription rtvDescription = { };
                    rtvDescription.type = ResourceViewType_RenderTarget;
                    rtvDescription.baseArrayLayer = 0;
                    rtvDescription.baseMipLevel = 0;
                    rtvDescription.dimension = ResourceViewDimension_2d;
                    rtvDescription.layerCount  = 1;
                    rtvDescription.mipLevelCount = 1;
                    rtvDescription.format = pSwapchain->getDesc().format;
                    GraphicsResource* swapchainResource = pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex());
                    pContext->transition(swapchainResource, ResourceState_RenderTarget);
                    ResourceViewId rtv = swapchainResource->asView(rtvDescription);
                    pContext->bindRenderTargets(1, &rtv);
                    Rect rect = { };
                    rect.x = 0;
                    rect.y = 0;
                    rect.width = pSwapchain->getDesc().renderWidth;
                    rect.height = pSwapchain->getDesc().renderHeight;
                    F32 color[4] = { 1.0f, 0.5f, 0.0f, 0 };
                    pContext->clearRenderTarget(0, color, rect);
                    pContext->transition(swapchainResource, ResourceState_Present);
                pContext->end();
                pSwapchain->present(pContext);
            }

            KeyboardListener listener;
            if (listener.isKeyDown(KeyCode_Escape))
            {
                pWindow->close();
            }

            pollEvents();
            
        }
    }
    pContext->wait();
    pDevice->destroySwapchain(pSwapchain);
    pDevice->releaseContext(pContext);
    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);
    
    pWindow->close();
    //pWindow2->close();
    Window::destroy(pWindow);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}