// 
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Graphics/CommandQueue.hpp"
#include "VulkanContext.hpp"

namespace Recluse {


class VulkanDevice;

struct QueueFamily;

// Vulkan queue implementation, inherits from GraphicsQueue API.
class VulkanQueue : public GraphicsQueue {
public:
    VulkanQueue(GraphicsQueueTypeFlags type) 
        : GraphicsQueue(type)
        , m_queue(nullptr)
        , m_pDevice(nullptr)
        , m_fence(VK_NULL_HANDLE)
        , m_pFamilyRef(nullptr) { }

    ~VulkanQueue();

    ErrType initialize(VulkanDevice* device, QueueFamily* pFamily, U32 queueFamilyIndex, U32 queueIndex);
    
    void destroy();

    ErrType submit(const QueueSubmit* payload) override;

    ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    ErrType copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
        CopyBufferRegion* pRegions, U32 numRegions) override;

    void wait() override;

    VkQueue operator()() const { return m_queue; }

    VkQueue get() const { return m_queue; }

private:
    // Generate an internal one-time only command buffer for copy operations.
    // Mainly a quick way to copy buffers and textures.
    VkCommandBuffer beginOneTimeCommandBuffer();


    VkQueue m_queue;
    VulkanDevice* m_pDevice;
    QueueFamily* m_pFamilyRef;
    VkFence m_fence;
};
} // Recluse