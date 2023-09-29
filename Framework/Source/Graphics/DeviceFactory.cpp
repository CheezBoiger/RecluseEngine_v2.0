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

#if defined RCL_DX11
#include "D3D11Backend/D3D11Instance.hpp"
#endif

namespace Recluse {


R_INTERNAL
GraphicsInstance* g_currentInstance = nullptr;


GraphicsInstance* GraphicsInstance::createInstance(enum GraphicsAPI api)
{
    if (g_currentInstance)
    {
        R_ERROR("Graphics", "An instance already exists for this application! Be sure to destroy the current instance, before creating a new one!");
        return g_currentInstance;
    }
    switch (api) 
    {
#if defined RCL_DX12
        case GraphicsApi_Direct3D12:
        {
            R_DEBUG("Graphics", "Creating D3D12 instance...");
            D3D12::D3D12Instance* ins = rlsMalloc<D3D12::D3D12Instance>();
            g_currentInstance = ins;
            return ins;
        } 
#endif
#if defined RCL_VULKAN
        case GraphicsApi_Vulkan:
        { 
            R_DEBUG("Graphics", "Creating Vulkan instance...");
            Vulkan::VulkanInstance* ins = rlsMalloc<Vulkan::VulkanInstance>();
            g_currentInstance = ins;
            return ins;
        }
#endif
#if defined RCL_DX11
        case GraphicsApi_Direct3D11:
        {
            R_NO_IMPL();
            R_ERROR("Graphics", "D3D11 is not fully supported yet!");
            return nullptr;
        }
#endif
        case GraphicsApi_OpenGL:
        case GraphicsApi_SoftwareRasterizer:
        case GraphicsApi_SoftwareRaytracer:
        default:
            R_ERROR("Graphics", "Unsupported graphics api. Can not create instance!");

    }
    return nullptr;
}


ResultCode GraphicsInstance::destroyInstance(GraphicsInstance* pInstance)
{
    if (!pInstance) 
    {
        R_ERROR("Graphics", "Null pointer passed to %s", __FUNCTION__);
        return RecluseResult_NullPtrExcept;
    }

    // Instance must be the current one, in order to destroy properly.
    if (pInstance == g_currentInstance)
    {
        g_currentInstance = nullptr;
    }

    pInstance->destroy();
    rlsFree<GraphicsInstance>(pInstance);

    return RecluseResult_Ok;
}
} // Recluse