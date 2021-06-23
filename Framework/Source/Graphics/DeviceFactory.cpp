//
#include "Graphics/GraphicsContext.hpp"
#include "Core/Messaging.hpp"
#include "Core/Memory/MemoryPool.hpp"

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
            new (ctx) VulkanContext;
            return ctx;
        }
        default:
            R_ERR("Graphics", "Unsupported graphics api. Can not create context!");

    }

    return nullptr;
}
}