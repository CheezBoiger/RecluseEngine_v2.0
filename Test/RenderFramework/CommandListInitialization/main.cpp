#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"


using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    GraphicsInstance* pInstance = GraphicsInstance::createInstance(GraphicsApi_Vulkan);

    if (!pInstance) {
        goto Exit;
    }
    
    ApplicationInfo appInfo = { };

    appInfo.appMajor    = 0;
    appInfo.appMinor    = 0;
    appInfo.appName     = "CommandListInitialization";
    appInfo.appPatch    = 0;
    appInfo.engineMajor = 0;
    appInfo.engineMinor = 0;
    appInfo.engineName  = "None";
    appInfo.enginePatch = 0;

    LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;

    ResultCode result = pInstance->initialize(appInfo, flags);

    if (result != RecluseResult_Ok) {
        R_ERROR("Test", "Failed to create instance!");
        goto Exit;
    }

    std::vector<GraphicsAdapter*>& adapters = pInstance->getGraphicsAdapters();

    for (GraphicsAdapter* adapter : adapters) {

        AdapterInfo adapterInfo = { }; 
        adapter->getAdapterInfo(&adapterInfo);

        R_INFO("Graphics", "\tDevice Name: %s\n\t\tVendor ID: %d\n", adapterInfo.deviceName, adapterInfo.vendorId);

    }

    DeviceCreateInfo deviceCreate   = { };
    GraphicsDevice* pDevice         = nullptr;
    GraphicsContext* pContext       = nullptr;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != RecluseResult_Ok) {
    
        R_ERROR("Graphics", "Failed to create device!");

    }

    pContext = pDevice->createContext();
    pContext->setFrames(2);
    GraphicsContext* context = nullptr;
    
    if (result != RecluseResult_Ok) {
    
        R_ERROR("Graphics", "Failed to create command list!!");
    
    } else {
    
        R_TRACE("Graphics", "Successfully created command list!");
        context = pContext;
        context->begin();
        context->end();

    }

    context->wait();
    pDevice->releaseContext(pContext);
    adapters[0]->destroyDevice(pDevice);

    GraphicsInstance::destroyInstance(pInstance);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}