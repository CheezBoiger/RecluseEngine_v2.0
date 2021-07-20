// 
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Graphics/CommandQueue.hpp"
#include "VulkanContext.hpp"

namespace Recluse {


class VulkanDevice;
class QueueFamily;

// Vulkan queue implementation, inherits from GraphicsQueue API.
class VulkanQueue : public GraphicsQueue {
public:
    VulkanQueue(GraphicsQueueTypeFlags type) 
        : GraphicsQueue(type)
        , m_queue(nullptr) { }

    ~VulkanQueue();

    ErrType initialize(VulkanDevice* device, QueueFamily* pFamily, U32 queueFamilyIndex, U32 queueIndex);
    
    void destroy();

    ErrType submit(const QueueSubmit* payload) override;

    ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    void wait() override;

    VkQueue operator()() const { return m_queue; }

    VkQueue get() const { return m_queue; }

private:
    VkQueue m_queue;
    VulkanDevice* m_pDevice;
    QueueFamily* m_pFamilyRef;
    VkFence m_fence;
};
} // Recluse