// 
#pragma once

#include "VulkanContext.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include <vector>
#include <list>

namespace Recluse {


class VulkanAdapter;
class VulkanQueue;
class VulkanSwapchain;
class VulkanAllocator;
struct DeviceCreateInfo;


struct QueueFamily {
    U32                                     maxQueueCount;
    U32                                     queueFamilyIndex;
    U32                                     currentAvailableQueueIndex;
    std::vector<VkCommandPool>              commandPools;
    GraphicsQueueTypeFlags                  flags;
};


class VulkanDevice : public GraphicsDevice {
public:
    VulkanDevice()
        : m_device(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_windowHandle(nullptr)
        , m_adapter(nullptr)
        , m_bufferCount(0)
        , m_currentBufferIndex(0) { 
        for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) { 
            m_bufferPool[i].memory = VK_NULL_HANDLE;
            m_imagePool[i].memory = VK_NULL_HANDLE;
        }
    }

    ErrType initialize(VulkanAdapter* iadapter, DeviceCreateInfo& info);

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, 
        const SwapchainCreateDescription& pDesc) override;

    ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) override;

    ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc) override;

    ErrType destroyCommandQueue(GraphicsQueue* pQueue) override;

    ErrType destroyResource(GraphicsResource* pResource) override;

    ErrType createCommandList(GraphicsCommandList** pList, GraphicsQueueTypeFlags flags) override;

    ErrType destroyCommandList(GraphicsCommandList* pList) override;

    ErrType createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc) override;

    void destroy(VkInstance instance);

    ErrType destroyResourceView(GraphicsResourceView* pResourceView) override;

    VkDevice operator()() {
        return m_device;
    }

    VkDevice get() const {
        return m_device;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    VulkanAdapter* getAdapter() const { return m_adapter; }

    VulkanAllocator* getBufferAllocator(ResourceMemoryUsage usage) const
        { return m_bufferAllocators[usage]; }

    VulkanAllocator* getImageAllocator(ResourceMemoryUsage usage) const 
        { return m_imageAllocators[usage]; }

    const std::vector<QueueFamily>& getQueueFamilies() const { return m_queueFamilies; }

    U32 getBufferCount() const { return m_bufferCount; }
    U32 getCurrentBufferIndex() const { return m_currentBufferIndex; }
    VkFence getCurrentFence() const { return m_fences[m_currentBufferIndex]; }
    
    // Increment the buffer index.
    inline void incrementBufferIndex() {
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount;
    }

    void prepare();

private:

    ErrType createSurface(VkInstance instance, void* handle);
    ErrType createCommandPools(U32 buffered);
    void createFences(U32 buffered);

    void destroyFences();
    void destroyCommandPools();

    VulkanAdapter* m_adapter;

    VkDevice m_device;
    VkSurfaceKHR m_surface;

    std::vector<QueueFamily> m_queueFamilies;
    std::list<VulkanQueue*> m_queues;
    std::list<VulkanSwapchain*> m_swapchains;
    std::vector<VkFence>        m_fences;

    VulkanMemoryPool m_bufferPool[RESOURCE_MEMORY_USAGE_COUNT];
    VulkanMemoryPool m_imagePool[RESOURCE_MEMORY_USAGE_COUNT];
    VulkanAllocator* m_bufferAllocators[RESOURCE_MEMORY_USAGE_COUNT];
    VulkanAllocator* m_imageAllocators[RESOURCE_MEMORY_USAGE_COUNT];

    // buffer count 
    U32 m_bufferCount;
    U32 m_currentBufferIndex;
    
    void* m_windowHandle;
};
} // Recluse