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

    Window* pWindow = Window::create(u8"SwapchainInitialization", 0, 0, 128, 128);

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

    EnableLayerFlags flags = LayerFeature_DebugValidationBit;

    ErrType result = pInstance->initialize(appInfo, flags);

    if (result != RecluseResult_Ok) {
        R_ERR("Test", "Failed to create context!");
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
    deviceCreate.buffering = 2;
    deviceCreate.swapchainDescription.buffering                    = FrameBuffering_Triple;
    deviceCreate.swapchainDescription.desiredFrames                = 3;
    deviceCreate.swapchainDescription.renderHeight                 = 128;
    deviceCreate.swapchainDescription.renderWidth                  = 128;
    deviceCreate.swapchainDescription.format                       = ResourceFormat_B8G8R8A8_Unorm;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != RecluseResult_Ok) {
    
        R_ERR("Graphics", "Failed to create device!");

    }

    pSwapchain = pDevice->getSwapchain();
    
    if (!pSwapchain) {
    
        R_ERR("Graphics", "Failed to create swapchain in test!");
    
    } else {
    
        R_TRACE("Graphics", "Succeeded swapchain creation!");

        pWindow->open();
        while (!pWindow->shouldClose()) {
            RealtimeTick::updateWatch(1ull, 0);
            RealtimeTick tick = RealtimeTick::getTick(0);
            R_TRACE("Graphics", "FPS: %f", 1.f / tick.delta());
            pSwapchain->present();

            pollEvents();
                    
        }
    }

    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);
    
    pWindow->close();
    Window::destroy(pWindow);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}