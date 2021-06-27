#include "Core/Messaging.hpp"
#include "Graphics/GraphicsAdapter.hpp"
#include "Graphics/GraphicsContext.hpp"
#include "Graphics/GraphicsDevice.hpp"
#include "Graphics/CommandQueue.hpp"

#include "Core/System/Window.hpp"
#include "Core/System/Input.hpp"

using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsContext* pContext       = GraphicsContext::createContext(GRAPHICS_API_VULKAN);
    GraphicsQueue* pQueue           = nullptr;

    Window* pWindow = Window::create(u8"SwapchainInitialization", 0, 0, 128, 128);

    if (!pContext || !pWindow) {
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

    EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

    ErrType result = pContext->initialize(appInfo, flags);

    if (result != REC_RESULT_OK) {
        R_ERR("Test", "Failed to create context!");
        goto Exit;
    }

    std::vector<GraphicsAdapter*>& adapters = pContext->getGraphicsAdapters();

    for (GraphicsAdapter* adapter : adapters) {

        AdapterInfo adapterInfo = { }; 
        adapter->getAdapterInfo(&adapterInfo);

        R_INFO("Graphics", "\tDevice Name: %s\n\t\tVendor ID: %d\n", adapterInfo.deviceName, adapterInfo.vendorId);

    }

    DeviceCreateInfo deviceCreate   = { };
    deviceCreate.winHandle = pWindow->getNativeHandle();

    result = adapters[0]->createDevice(&deviceCreate, &pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create device!");

    }

    result = pDevice->createCommandQueue(&pQueue, QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_GRAPHICS);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create the presentation queue!");
    
    }

    SwapchainCreateDescription scInfo   = { };
    scInfo.desiredFrames                = 3;
    scInfo.renderHeight                 = 128;
    scInfo.renderWidth                  = 128;
    scInfo.pBackbufferQueue             = pQueue;

    result = pDevice->createSwapchain(&pSwapchain, &scInfo);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create swapchain in test!");
    
    } else {
    
        R_TRACE("Graphics", "Succeeded swapchain creation!");

        pWindow->open();

        while (!pWindow->shouldClose()) {

            pSwapchain->present();

            pollEvents();
                    
        }

        pDevice->destroySwapchain(pSwapchain);
        pDevice->destroyCommandQueue(pQueue);
    }

    adapters[0]->destroyDevice(pDevice, pContext);

    pContext->destroy();
    
    pWindow->close();
    Window::destroy(pWindow);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}