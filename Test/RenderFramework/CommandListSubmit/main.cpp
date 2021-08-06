#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/CommandQueue.hpp"
#include "Recluse/Graphics/CommandList.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/RealtimeTick.hpp"
#include "Recluse/System/Input.hpp"

using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();

    GraphicsSwapchain* pSwapchain   = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    GraphicsInstance* pInstance      = GraphicsInstance::createInstance(GRAPHICS_API_VULKAN);
    GraphicsQueue* pQueue           = nullptr;

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

    deviceCreate.buffering = 2;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create device!");

    }

    result = pDevice->createCommandQueue(&pQueue, QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_PRESENT);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create the presentation queue!");
    
    }

    SwapchainCreateDescription scInfo   = { };
    scInfo.buffering                    = FRAME_BUFFERING_TRIPLE;
    scInfo.desiredFrames                = 3;
    scInfo.renderHeight                 = 128;
    scInfo.renderWidth                  = 128;
    scInfo.pBackbufferQueue             = pQueue;

    result = pDevice->createSwapchain(&pSwapchain, scInfo);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create swapchain in test!");
    
    } else {
    
        R_TRACE("Graphics", "Succeeded swapchain creation!");

        pWindow->open();
        
        GraphicsCommandList* pList = nullptr;
        GraphicsCommandList* pList2 = nullptr;
        
        result = pDevice->createCommandList(&pList, QUEUE_TYPE_GRAPHICS);
        result = pDevice->createCommandList(&pList2, QUEUE_TYPE_GRAPHICS);

        if (result != REC_RESULT_OK) {
        
            R_ERR("Graphics", "Failed to create command list...")
        
        }

        while (!pWindow->shouldClose()) {
            RealtimeTick tick = RealtimeTick::getTick();
            R_TRACE("Graphics", "FPS: %f", 1.f / tick.getDeltaTimeS());

            pList->begin();
            pList->end();

            pList2->begin();
            pList2->end();

            QueueSubmit submit      = { };
            submit.numCommandLists  = 1;
            submit.pCommandLists    = &pList;

            pQueue->submit(&submit);

            submit.pCommandLists = &pList2;

            pQueue->submit(&submit);

            pSwapchain->present();

            pollEvents();
                    
        }

        // Wait until all command lists have been executed.
        pQueue->wait();

        pDevice->destroySwapchain(pSwapchain);
        pDevice->destroyCommandList(pList);
        pDevice->destroyCommandList(pList2);
        pDevice->destroyCommandQueue(pQueue);
    }

    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);
    
    pWindow->close();
    Window::destroy(pWindow);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}