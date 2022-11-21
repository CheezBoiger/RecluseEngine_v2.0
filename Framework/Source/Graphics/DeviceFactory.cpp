//
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

// Only one is supported.
#if defined RCL_VULKAN
#include "VulkanBackend/VulkanInstance.hpp"
#endif

#if defined RCL_DX12
#include "D3D12Backend/D3D12Instance.hpp"
#endif

namespace Recluse {


GraphicsInstance* GraphicsInstance::createInstance(enum GraphicsAPI api)
{
    switch (api) 
    {
#if defined RCL_DX12
        case GraphicsApi_Direct3D12:
        {
            R_DEBUG("Graphics", "Creating D3D12 instance...");
            D3D12Instance* ins = rlsMalloc<D3D12Instance>();
            return ins;
        } 
#endif
#if defined RCL_VULKAN
        case GraphicsApi_Vulkan:

        { 
            R_DEBUG("Graphics", "Creating Vulkan instance...");
            VulkanInstance* ins = rlsMalloc<VulkanInstance>();
            return ins;
        }
#endif
        case GraphicsApi_OpenGL:
        case GraphicsApi_Direct3D11:
        case GraphicsApi_SoftwareRasterizer:
        case GraphicsApi_SoftwareRaytracer:
        default:
            R_ERR("Graphics", "Unsupported graphics api. Can not create instance!");

    }
    return nullptr;
}


ErrType GraphicsInstance::destroyInstance(GraphicsInstance* pInstance)
{
    if (!pInstance) 
    {
        R_ERR("Graphics", "Null pointer passed to %s", __FUNCTION__);
        return RecluseResult_NullPtrExcept;
    }

    pInstance->destroy();
    rlsFree<GraphicsInstance>(pInstance);

    return RecluseResult_Ok;
}
} // Recluse