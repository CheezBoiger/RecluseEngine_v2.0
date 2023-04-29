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

    GraphicsInstance* pInstance = GraphicsInstance::createInstance(GraphicsApi_Vulkan);

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

    LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;

    ResultCode result = pInstance->initialize(appInfo, flags);

    if (result != RecluseResult_Ok) {
        R_ERROR("Test", "Failed to create context!");
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
    
        R_ERROR("Graphics", "Failed to create device!");

    }
    
    MemoryReserveDescription memReserves = { };
    memReserves.bufferPools[ResourceMemoryUsage_CpuToGpu] = 32ull * R_1KB;
    memReserves.bufferPools[ResourceMemoryUsage_GpuToCpu] = 32ull * R_1KB;
    memReserves.bufferPools[ResourceMemoryUsage_CpuOnly] = 256 * R_1MB;
    memReserves.texturePoolGPUOnly = 512 * R_1MB; // half a GB.
    memReserves.bufferPools[ResourceMemoryUsage_GpuOnly] = 512 * R_1MB; // half a GB.
    
    pDevice->reserveMemory(memReserves);

    GraphicsResource* pTexture          = nullptr;
    GraphicsResourceView* pView         = nullptr;
    GraphicsResourceDescription desc    = { };

    desc.usage          = ResourceUsage_RenderTarget | ResourceUsage_ShaderResource;
    desc.memoryUsage    = ResourceMemoryUsage_GpuOnly;
    desc.width          = 128;
    desc.height         = 128;
    desc.depthOrArraySize = 1;
    desc.mipLevels      = 8;
    desc.dimension      = ResourceDimension_2d;
    desc.samples        = 1;
    desc.format         = ResourceFormat_R8G8B8A8_Unorm;

    result = pDevice->createResource(&pTexture, desc, ResourceState_ShaderResource);

    if (result != RecluseResult_Ok) {
    
        R_ERROR("Graphics", "Failed to create texture!");

    } else {
    
        R_TRACE("Graphics", "Successfully created texture!");
        ResourceViewDescription viewDesc   = { };
        viewDesc.baseArrayLayer     = 0;
        viewDesc.baseMipLevel       = 0;
        viewDesc.dimension          = ResourceViewDimension_2d;
        viewDesc.format             = ResourceFormat_R8G8B8A8_Unorm;
        viewDesc.layerCount         = 1;
        viewDesc.mipLevelCount      = 1;
        viewDesc.pResource          = pTexture;
        viewDesc.type               = ResourceViewType_RenderTarget;
       
        result = pDevice->createResourceView(&pView, viewDesc);
        
        if (result != RecluseResult_Ok) {
        
            R_ERROR("Graphics", "Failed to create view...");
        
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