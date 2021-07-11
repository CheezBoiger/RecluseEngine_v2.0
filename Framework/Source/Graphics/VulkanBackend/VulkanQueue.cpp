//
#include "VulkanQueue.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


VulkanQueue::~VulkanQueue()
{
    destroy();
}


ErrType VulkanQueue::initialize(VkDevice device, U32 queueFamilyIndex, U32 queueIndex)
{
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &m_queue);
    return REC_RESULT_OK;
}


void VulkanQueue::wait()
{
    VkResult result = vkQueueWaitIdle(m_queue);
    
    if (result != VK_SUCCESS) {
    
        R_WARN(R_CHANNEL_VULKAN, "Failed to wait for queue idle!");
    
    }
}


void VulkanQueue::destroy()
{
    if (m_queue) {

        R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan queue handle...");

        m_queue = VK_NULL_HANDLE;

    }
}


ErrType VulkanQueue::submit(const QueueSubmit* payload)
{
    return REC_RESULT_OK;
}
} // Recluse