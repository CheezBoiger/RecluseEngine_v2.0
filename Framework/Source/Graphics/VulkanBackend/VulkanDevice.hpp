// 
#pragma once

#include "VulkanContext.hpp"
#include "Graphics/GraphicsDevice.hpp"

#include <vector>
#include <list>

namespace Recluse {


class VulkanAdapter;
class VulkanQueue;
class VulkanSwapchain;
class VulkanAllocator;
struct DeviceCreateInfo;


struct QueueFamily {
    U32                     maxQueueCount;
    U32                     queueFamilyIndex;
    U32                     currentAvailableQueueIndex;
    GraphicsQueueTypeFlags  flags;
};


class VulkanDevice : public GraphicsDevice {
public:
    VulkanDevice()
        : m_device(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_windowHandle(nullptr) { 
        for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) 
            m_deviceMemory[i] = VK_NULL_HANDLE;
    }

    ErrType initialize(VulkanAdapter* iadapter, DeviceCreateInfo& info);

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, 
        const SwapchainCreateDescription& pDesc) override;

    ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) override;

    ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc) override;

    ErrType destroyCommandQueue(GraphicsQueue* pQueue) override;

    void destroy(VkInstance instance);

    VkDevice operator()() {
        return m_device;
    }

    VkDevice get() const {
        return m_device;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    VulkanAdapter* getAdapter() const { return m_adapter; }

    VulkanAllocator* getAllocator(ResourceMemoryUsage usage) const
        { return m_allocators[usage]; }

private:

    ErrType createSurface(VkInstance instance, void* handle);

    VulkanAdapter* m_adapter;

    VkDevice m_device;
    VkSurfaceKHR m_surface;

    std::vector<QueueFamily> m_queueFamilies;
    std::list<VulkanQueue*> m_queues;
    std::list<VulkanSwapchain*> m_swapchains;

    VkDeviceMemory m_deviceMemory[RESOURCE_MEMORY_USAGE_COUNT];
    VulkanAllocator* m_allocators[RESOURCE_MEMORY_USAGE_COUNT];
    
    void* m_windowHandle;
};
} // Recluse