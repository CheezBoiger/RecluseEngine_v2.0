// 
#pragma once

#include "Recluse/Types.hpp"

#include "VulkanInstance.hpp"

namespace Recluse {
namespace Vulkan {

class VulkanDevice;
class VulkanAdapter;
struct QueueFamily;

// Vulkan queue implementation, inherits from GraphicsQueue API.
class VulkanQueue 
{
public:
    static const U32 failedIndex = 0xffffffff;
    static void             generateCopyResource(VkCommandBuffer commandBuffer, GraphicsResource* dst, GraphicsResource* src);

    VulkanQueue(VkQueueFlags type = 0) 
        : m_queue(nullptr)
        , m_pDevice(nullptr)
        , m_fence(VK_NULL_HANDLE)
        , m_oneTimeOnlyFence(VK_NULL_HANDLE)
        , m_pFamilyRef(nullptr)
        , m_tempCommandPool(VK_NULL_HANDLE)
        , m_queueFlags(type) { }

    ~VulkanQueue();

    ResultCode              initialize(VulkanDevice* device, QueueFamily* pFamily, U32 queueIndex);
    void                    destroy();

    //ErrType submit(const QueueSubmit* payload);
    ResultCode              copyResource(GraphicsResource* dst, GraphicsResource* src);
    ResultCode              copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
                                const CopyBufferRegion* pRegions, U32 numRegions);

    void                    wait();
    VkQueue                 operator()() const { return m_queue; }
    VkQueue                 get() const { return m_queue; }
    B32                     isPresentSupported(VulkanAdapter* pAdapter, VkSurfaceKHR surface) const;
    VkQueueFlags            getQueueFlags() const { return m_queueFlags; }
    QueueFamily*            getFamily() const { return m_pFamilyRef; }

    // Generate an internal one-time only command buffer for copy operations.
    // Mainly a quick way to copy buffers and textures.
    VkCommandBuffer         beginOneTimeCommandBuffer();
    ResultCode              endAndSubmitOneTimeCommandBuffer(VkCommandBuffer commandBuffer);

    // Obtain the temporary command pool from this queue, any and all command buffers created by this pool, must be freed prior to 
    // the release of this queue!
    VkCommandPool           getTemporaryCommandPool() { return m_tempCommandPool; }

private:

    void                    freeOneTimeOnlyCommandBuffer(VkCommandBuffer commandBuffer);

    VkQueue                 m_queue;
    VulkanDevice*           m_pDevice;
    QueueFamily*            m_pFamilyRef;
    VkFence                 m_fence;
    VkFence                 m_oneTimeOnlyFence;
    VkQueueFlags            m_queueFlags;
    VkCommandPool           m_tempCommandPool;
};
} // Vulkan
} // Recluse