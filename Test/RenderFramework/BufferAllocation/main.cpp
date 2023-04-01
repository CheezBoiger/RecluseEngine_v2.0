#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/Resource.hpp"

#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

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
    appInfo.appName     = "BufferAllocation";
    appInfo.appPatch    = 0;
    appInfo.engineMajor = 0;
    appInfo.engineMinor = 0;
    appInfo.engineName  = "None";
    appInfo.enginePatch = 0;

    LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;

    ErrType result = pInstance->initialize(appInfo, flags);

    if (result != RecluseResult_Ok) {
        R_ERR("Test", "Failed to create instance!");
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

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != RecluseResult_Ok) {
    
        R_ERR("Graphics", "Failed to create device!");

    }
    
    MemoryReserveDescription memReserves = { };
    memReserves.bufferPools[ResourceMemoryUsage_CpuToGpu] = 32ull * R_1KB;
    memReserves.bufferPools[ResourceMemoryUsage_GpuToCpu] = 32ull * R_1KB;
    memReserves.bufferPools[ResourceMemoryUsage_CpuOnly] = 256 * R_1MB;
    memReserves.texturePoolGPUOnly = 512 * R_1MB; // half a GB.
    memReserves.bufferPools[ResourceMemoryUsage_GpuOnly] = 512 * R_1MB; // half a GB.
    
    pDevice->reserveMemory(memReserves);

    GraphicsResource* pBuffer = nullptr;
    GraphicsResource* pBuffer2 = nullptr;
    
    GraphicsResourceDescription bufferDesc = { };
    bufferDesc.usage        = ResourceUsage_ConstantBuffer;
    bufferDesc.dimension    = ResourceDimension_Buffer;
    bufferDesc.memoryUsage  = ResourceMemoryUsage_GpuOnly;
    bufferDesc.width        = R_1KB * 1024ull;

    result = pDevice->createResource(&pBuffer, bufferDesc, ResourceState_ConstantBuffer);

    if (result != RecluseResult_Ok) {
    
        R_ERR("Graphics", "Failed to create buffer!");
    
    } else {
        
        R_TRACE("Graphics", "Successfully create buffer!");
        
        result = pDevice->createResource(&pBuffer2, bufferDesc, ResourceState_ConstantBuffer);

        if (result != RecluseResult_Ok) {
        
            R_ERR("Graphics", "Failed to create second buffer!!");
        
        } else {
    
            R_TRACE("Graphics", "Successfully created second buffer!!");
            pDevice->destroyResource(pBuffer2);    

        }

        pDevice->destroyResource(pBuffer);
    }

    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}