// 
#pragma once

#include "Recluse/Types.hpp"

#include "VulkanInstance.hpp"

namespace Recluse {


class VulkanDevice;

struct QueueFamily;

// Vulkan queue implementation, inherits from GraphicsQueue API.
class VulkanQueue 
{
public:

    static void generateCopyResource(VkCommandBuffer commandBuffer, GraphicsResource* dst, GraphicsResource* src);

    VulkanQueue(VkQueueFlags type, B32 isPresentSupported = false) 
        : m_queue(nullptr)
        , m_pDevice(nullptr)
        , m_fence(VK_NULL_HANDLE)
        , m_oneTimeOnlyFence(VK_NULL_HANDLE)
        , m_pFamilyRef(nullptr)
        , m_isPresentSupported(isPresentSupported)
        , m_tempCommandPool(VK_NULL_HANDLE)
        , m_queueFlags(type) { }

    ~VulkanQueue();

    ResultCode initialize(VulkanDevice* device, QueueFamily* pFamily, U32 queueFamilyIndex, U32 queueIndex);
    
    void destroy();

    //ErrType submit(const QueueSubmit* payload);

    void setTemporaryCommandPoolUse(VkCommandPool pool) { m_tempCommandPool = pool; }

    ResultCode copyResource(GraphicsResource* dst, GraphicsResource* src);

    ResultCode copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
        const CopyBufferRegion* pRegions, U32 numRegions);

    void wait();

    VkQueue operator()() const { return m_queue; }

    VkQueue get() const { return m_queue; }

    B32 isPresentSupported() const { return m_isPresentSupported; }
    
    VkQueueFlags getQueueFlags() const { return m_queueFlags; }

    // Generate an internal one-time only command buffer for copy operations.
    // Mainly a quick way to copy buffers and textures.
    VkCommandBuffer beginOneTimeCommandBuffer();

    ResultCode endAndSubmitOneTimeCommandBuffer(VkCommandBuffer commandBuffer);

private:

    void            freeOneTimeOnlyCommandBuffer(VkCommandBuffer commandBuffer);

    VkQueue         m_queue;
    VulkanDevice*   m_pDevice;
    QueueFamily*    m_pFamilyRef;
    VkFence         m_fence;
    VkFence         m_oneTimeOnlyFence;
    B32             m_isPresentSupported;
    VkQueueFlags    m_queueFlags;
    VkCommandPool   m_tempCommandPool;
};
} // Recluse