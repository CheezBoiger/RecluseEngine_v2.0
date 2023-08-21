#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/CommandList.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/System/Input.hpp"

using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsInstance* pInstance       = GraphicsInstance::createInstance(GraphicsApi_Direct3D12);

    Window* pWindow = Window::create(u8"SwapchainInitialization", 0, 0, 1280, 720);

    if (!pInstance || !pWindow) {
        goto Exit;
    }
    
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

    DeviceCreateInfo deviceCreate   = { };
    deviceCreate.winHandle = pWindow->getNativeHandle();
    deviceCreate.swapchainDescription.buffering                    = FrameBuffering_Triple;
    deviceCreate.swapchainDescription.desiredFrames                = 3;
    deviceCreate.swapchainDescription.renderHeight                 = 720;
    deviceCreate.swapchainDescription.renderWidth                  = 1280;
    deviceCreate.swapchainDescription.format                       = ResourceFormat_B8G8R8A8_Unorm;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != RecluseResult_Ok) {
    
        R_ERROR("Graphics", "Failed to create device!");

    }
    GraphicsContext* pContext = pDevice->createContext();
    pContext->setBuffers(2);
    pWindow->setToCenter();

    pSwapchain = pDevice->getSwapchain();
    
    if (!pSwapchain) {
    
        R_ERROR("Graphics", "Failed to create swapchain in test!");
    
    } else {
    
        R_TRACE("Graphics", "Succeeded swapchain creation!");

        pWindow->open();
        while (!pWindow->shouldClose()) 
        {
            RealtimeTick::updateWatch(1ull, 0);
            RealtimeTick tick = RealtimeTick::getTick(0);
            R_TRACE("Graphics", "FPS: %f", 1.f / tick.delta());
            pContext->begin();
                
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
            pSwapchain->present();

            pollEvents();
                    
        }
    }
    pContext->wait();
    pDevice->releaseContext(pContext);
    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);
    
    pWindow->close();
    Window::destroy(pWindow);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}