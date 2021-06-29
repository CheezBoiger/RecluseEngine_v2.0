#include "Core/Messaging.hpp"
#include "Graphics/GraphicsAdapter.hpp"
#include "Graphics/GraphicsContext.hpp"
#include "Graphics/GraphicsDevice.hpp"

#include "Core/Memory/MemoryPool.hpp"


using namespace Recluse;

int main(int c, char* argv[])
{
    Log::initializeLoggingSystem();

    GraphicsContext* pContext = GraphicsContext::createContext(GRAPHICS_API_VULKAN);

    if (!pContext) {
        goto Exit;
    }
    
    ApplicationInfo appInfo = { };

    appInfo.appMajor    = 0;
    appInfo.appMinor    = 0;
    appInfo.appName     = "MemoryInitialization";
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
    GraphicsDevice* pDevice         = nullptr;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create device!");

    }
    
    MemoryReserveDesc memReserves = { };
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_CPU_TO_GPU] = 32ull * R_1KB;
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_GPU_TO_CPU] = 32ull * R_1KB;
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY] = 256 * R_1MB;
    memReserves.texturePoolGPUOnly = 512 * R_1MB; // half a GB.
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY] = 512 * R_1MB; // half a GB.
    
    pDevice->reserveMemory(memReserves);

    adapters[0]->destroyDevice(pDevice);

    pContext->destroy();

Exit:
    Log::destroyLoggingSystem();
    return 0;
}