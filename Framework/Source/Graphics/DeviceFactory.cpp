//
#include "Recluse/Graphics/GraphicsContext.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

// Only one is supported.
#include "VulkanBackend/VulkanContext.hpp"

namespace Recluse {


GraphicsContext* GraphicsContext::createContext(enum GraphicsAPI api)
{
    switch (api) {

        case GRAPHICS_API_VULKAN: 
        { 
            R_DEBUG("Graphics", "Creating Vulkan context...");
            VulkanContext* ctx = rlsMalloc<VulkanContext>();
            return ctx;
        }
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