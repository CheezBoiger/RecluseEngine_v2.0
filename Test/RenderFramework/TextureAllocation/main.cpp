#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/ResourceView.hpp"

#include "Recluse//Memory/MemoryPool.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"


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
    appInfo.appName     = "TextureAllocation";
    appInfo.appPatch    = 0;
    appInfo.engineMajor = 0;
    appInfo.engineMinor = 0;
    appInfo.engineName  = "None";
    appInfo.enginePatch = 0;

    EnableLayerFlags flags = LAYER_FEATURE_DEBUG_VALIDATION_BIT;

    ErrType result = pInstance->initialize(appInfo, flags);

    if (result != R_RESULT_OK) {
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
    deviceCreate.buffering          = 3;
    GraphicsDevice* pDevice         = nullptr;

    result = adapters[0]->createDevice(deviceCreate, &pDevice);

    if (result != R_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create device!");

    }
    
    MemoryReserveDesc memReserves = { };
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_CPU_TO_GPU] = 32ull * R_1KB;
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_GPU_TO_CPU] = 32ull * R_1KB;
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY] = 256 * R_1MB;
    memReserves.texturePoolGPUOnly = 512 * R_1MB; // half a GB.
    memReserves.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY] = 512 * R_1MB; // half a GB.
    
    pDevice->reserveMemory(memReserves);

    GraphicsResource* pTexture          = nullptr;
    GraphicsResourceView* pView         = nullptr;
    GraphicsResourceDescription desc    = { };

    desc.usage          = RESOURCE_USAGE_RENDER_TARGET | RESOURCE_USAGE_SHADER_RESOURCE;
    desc.memoryUsage    = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    desc.width          = 128;
    desc.height         = 128;
    desc.depth          = 1;
    desc.arrayLevels    = 1;
    desc.mipLevels      = 8;
    desc.dimension      = RESOURCE_DIMENSION_2D;
    desc.samples        = 1;
    desc.format         = RESOURCE_FORMAT_R8G8B8A8_UNORM;

    result = pDevice->createResource(&pTexture, desc, RESOURCE_STATE_SHADER_RESOURCE);

    if (result != R_RESULT_OK) {
    
        R_ERR("Graphics", "Failed to create texture!");

    } else {
    
        R_TRACE("Graphics", "Successfully created texture!");
        ResourceViewDesc viewDesc   = { };
        viewDesc.baseArrayLayer     = 0;
        viewDesc.baseMipLevel       = 0;
        viewDesc.dimension          = RESOURCE_VIEW_DIMENSION_2D;
        viewDesc.format             = RESOURCE_FORMAT_R8G8B8A8_UNORM;
        viewDesc.layerCount         = 1;
        viewDesc.mipLevelCount      = 1;
        viewDesc.pResource          = pTexture;
        viewDesc.type               = RESOURCE_VIEW_TYPE_RENDER_TARGET;
       
        result = pDevice->createResourceView(&pView, viewDesc);
        
        if (result != R_RESULT_OK) {
        
            R_ERR("Graphics", "Failed to create view...");
        
        } else {
        
            R_TRACE("Graphics", "Successfully created view...");    
            pDevice->destroyResourceView(pView);

        }

        pDevice->destroyResource(pTexture);
    
    }
    
    adapters[0]->destroyDevice(pDevice);

   GraphicsInstance::destroyInstance(pInstance);

Exit:
    Log::destroyLoggingSystem();
    return 0;
}