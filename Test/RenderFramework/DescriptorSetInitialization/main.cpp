//
#include "Recluse/Messaging.hpp"
#include "Recluse/RealtimeTick.hpp"

#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/CommandQueue.hpp"
#include "Recluse/Graphics/RenderPass.hpp"

#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"

using namespace Recluse;

struct Vec4 {
    F32 position[4];
    F32 normal[3];
    F32 pad0;
    F32 texcoord0[2];
    F32 texcoord1[2];
};

struct VertexData {
    
};

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();
    RealtimeTick::initialize();

    GraphicsContext* pContext       = nullptr;
    GraphicsAdapter* pAdapter       = nullptr;
    GraphicsDevice* pDevice         = nullptr;
    Window* pWindow                 = nullptr;
    GraphicsCommandList* pList      = nullptr;
    DescriptorSetLayout* pLayout    = nullptr;
    GraphicsResource* pData         = nullptr;
    GraphicsQueue* pQueue           = nullptr;
    DescriptorSet*  pSet            = nullptr;
    GraphicsSwapchain* pSwapchain   = nullptr;
    RenderPass* pRenderPass         = nullptr;
    ErrType result                  = REC_RESULT_OK;

    pWindow = Window::create(u8"DescriptorSetInitialization", 0, 0, 1024, 1024);

    pContext = GraphicsContext::createContext(GRAPHICS_API_VULKAN);

    if (!pContext) {

        R_ERR("TEST", "Failed to create graphics context!");

    }

    {
        ApplicationInfo appInfo = { };
        appInfo.appMajor = 0;
        appInfo.appMinor = 0;
        appInfo.appPatch = 0;
        appInfo.appName = "DescriptorSetInitialization";
        appInfo.engineMajor = 0;
        appInfo.engineMinor = 0;
        appInfo.engineName = "DescriptorSetInitialization";

        EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

        result = pContext->initialize(appInfo, flags);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to initialize context!");    

    }

    std::vector<GraphicsAdapter*> adapters = pContext->getGraphicsAdapters();
    pAdapter = adapters[0];

    {
        DeviceCreateInfo info = { };
        info.buffering = 2;
        info.winHandle = pWindow->getNativeHandle();
        result = pAdapter->createDevice(info, &pDevice);
    }

    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create device from adapter!");
    
    }

    result = pDevice->createCommandQueue(&pQueue, QUEUE_TYPE_GRAPHICS | QUEUE_TYPE_PRESENT);

    if (result != REC_RESULT_OK) {

        R_ERR("TEST", "Failed to create device queue!");

    }

    {
        SwapchainCreateDescription info = { };
        info.buffering = FRAME_BUFFERING_DOUBLE;
        info.desiredFrames = 2;
        info.pBackbufferQueue = pQueue;
        info.renderWidth = 1024;
        info.renderHeight = 1024;
        result = pDevice->createSwapchain(&pSwapchain, info);
    }

    if (result != REC_RESULT_OK) {

        R_ERR("TEST", "FAILED TO CREATE SWAPCHAIN!!");    

    }

    result = pDevice->createCommandList(&pList, QUEUE_TYPE_GRAPHICS);
    
    if (result != REC_RESULT_OK) {
    
        R_ERR("TEST", "Failed to create command list!");
    
    }
    
    pWindow->open();

    // TODO: Still needs more work!!

    // Enter game loop.
    R_TRACE("TEST", "Entering game loop...");

    while (!pWindow->shouldClose()) {
        pList->begin();
            //pList->setRenderPass(pRenderPass);
        pList->end();

        pSwapchain->present();
        pollEvents();
    }

    R_TRACE("TEST", "Exited game loop...");
    pQueue->wait();    

    pDevice->destroyCommandList(pList);
    pDevice->destroySwapchain(pSwapchain);
    pDevice->destroyCommandQueue(pQueue);    
    pAdapter->destroyDevice(pDevice);
    Window::destroy(pWindow);
    GraphicsContext::destroyContext(pContext);
    Log::destroyLoggingSystem();
    return 0;
}