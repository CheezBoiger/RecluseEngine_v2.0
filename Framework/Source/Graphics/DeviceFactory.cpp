//
#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

// Only one is supported.
#if defined RCL_VULKAN
#include "VulkanBackend/VulkanContext.hpp"
#endif

#if defined RCL_DX12
#include "D3D12Backend/D3D12Context.hpp"
#endif

namespace Recluse {


GraphicsContext* GraphicsContext::createContext(enum GraphicsAPI api)
{
    switch (api) {
#if defined RCL_DX12
        case GRAPHICS_API_D3D12:
        {
            R_DEBUG("Graphics", "Creating D3D12 context...");
            D3D12Context* ctx = rlsMalloc<D3D12Context>();
            return ctx;
        } 
#endif
#if defined RCL_VULKAN
        case GRAPHICS_API_VULKAN:

        { 
            R_DEBUG("Graphics", "Creating Vulkan context...");
            VulkanContext* ctx = rlsMalloc<VulkanContext>();
            return ctx;
        }
#endif
        default:
            R_ERR("Graphics", "Unsupported graphics api. Can not create context!");

    }
    return nullptr;
}


ErrType GraphicsContext::destroyContext(GraphicsContext* pContext)
{
    if (!pContext) {

        R_ERR("Graphics", "Null pointer passed to %s", __FUNCTION__);    

        return REC_RESULT_NULL_PTR_EXCEPTION;
    
    }

    pContext->destroy();
    rlsFree<GraphicsContext>(pContext);

    return REC_RESULT_OK;
}
} // Recluse