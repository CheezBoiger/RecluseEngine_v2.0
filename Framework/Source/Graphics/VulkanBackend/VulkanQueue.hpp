// 
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Graphics/CommandQueue.hpp"
#include "VulkanContext.hpp"

namespace Recluse {


class VulkanDevice;


// Vulkan queue implementation, inherits from GraphicsQueue API.
class VulkanQueue : public GraphicsQueue {
public:
    VulkanQueue(GraphicsQueueTypeFlags type) 
        : GraphicsQueue(type)
        , m_queue(nullptr) { }

    ~VulkanQueue();

    ErrType initialize(VulkanDevice* device, U32 queueFamilyIndex, U32 queueIndex);
    
    void destroy();

    ErrType submit(const QueueSubmit* payload) override;

    void wait() override;

    VkQueue operator()() const { return m_queue; }

    VkQueue get() const { return m_queue; }

private:
    VkQueue m_queue;
    VulkanDevice* m_pDevice;
};
} // Recluse