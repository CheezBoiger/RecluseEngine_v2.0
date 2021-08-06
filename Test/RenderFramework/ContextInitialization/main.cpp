#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"


using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    GraphicsInstance* pInstance = GraphicsInstance::createInstance(GRAPHICS_API_VULKAN);

    if (!pInstance) {
        goto Exit;
    }
    
    ApplicationInfo appInfo = { };

    appInfo.appMajor    = 0;
    appInfo.appMinor    = 0;
    appInfo.appName     = "ContextInitialization";
    appInfo.appPatch    = 0;
    appInfo.engineMajor = 0;
    appInfo.engineMinor = 0;
    appInfo.engineName  = "None";
    appInfo.enginePatch = 0;

    EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

    ErrType result = pInstance->initialize(appInfo, flags);

    if (result != REC_RESULT_OK) {
        R_ERR("Test", "Failed to create instance!");
        goto Exit;
    }

    std::vector<GraphicsAdapter*>& adapters = pInstance->getGraphicsAdapters();

    for (GraphicsAdapter* adapter : adapters) {

        AdapterInfo adapterInfo = { }; 
        adapter->getAdapterInfo(&adapterInfo);

        R_INFO("Graphics", "\tDevice Name: %s\n\t\tVendor ID: %d\n\t\tVendor Name: %s\n", adapterInfo.deviceName, 
            adapterInfo.vendorId, adapterInfo.vendorName);

    }

    DeviceCreateInfo deviceCreate   = { };
    GraphicsDevice* pDevice         = nullptr;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create device!");

    }

    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}