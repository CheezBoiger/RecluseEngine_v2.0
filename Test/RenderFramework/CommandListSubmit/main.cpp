#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/CommandList.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/RealtimeTick.hpp"
#include "Recluse/System/Input.hpp"

using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);

    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsInstance* pInstance      = GraphicsInstance::createInstance(GRAPHICS_API_VULKAN);

    Window* pWindow = Window::create(u8"CommandListSubmit", 0, 0, 128, 128);

    if (!pInstance || !pWindow) {
        goto Exit;
    }
    
    ApplicationInfo appInfo = { };

    appInfo.appMajor    = 0;
    appInfo.appMinor    = 0;
    appInfo.appName     = "CommandListSubmit";
    appInfo.appPatch    = 0;
    appInfo.engineMajor = 0;
    appInfo.engineMinor = 0;
    appInfo.engineName  = "None";
    appInfo.enginePatch = 0;

    EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

    ErrType result = pInstance->initialize(appInfo, flags);

    if (result != REC_RESULT_OK) {
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
    deviceCreate.swapchainDescription.buffering                    = FRAME_BUFFERING_TRIPLE;
    deviceCreate.swapchainDescription.desiredFrames                = 3;
    deviceCreate.swapchainDescription.renderHeight                 = 128;
    deviceCreate.swapchainDescription.renderWidth                  = 128;
    deviceCreate.buffering = 2;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create device!");

    }

    pSwapchain = pDevice->getSwapchain();
    
    if (!pSwapchain) {
    
        R_ERR("Graphics", "Failed to create swapchain in test!");
    
    } else {
    
        R_TRACE("Graphics", "Succeeded swapchain creation!");

        pWindow->open();
        
        GraphicsCommandList* pList = nullptr;
        GraphicsCommandList* pList2 = nullptr;
        
        pList = pDevice->getCommandList();

        while (!pWindow->shouldClose()) {
            RealtimeTick::updateWatch(1ull, 0);
            RealtimeTick tick = RealtimeTick::getTick(0);
            R_TRACE("Graphics", "FPS: %f", 1.f / tick.getDeltaTimeS());

            pList->begin();
            pList->end();
            pSwapchain->present();
            pollEvents();
                    
        }

        // Wait until all command lists have been executed.
        pDevice->wait();
    }

    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);
    
    pWindow->close();
    Window::destroy(pWindow);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}