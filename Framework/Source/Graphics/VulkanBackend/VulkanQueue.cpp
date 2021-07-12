//
#include "VulkanQueue.hpp"
#include "VulkanDevice.hpp"

#include "VulkanCommandList.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


VulkanQueue::~VulkanQueue()
{
    destroy();
}


ErrType VulkanQueue::initialize(VulkanDevice* device, U32 queueFamilyIndex, U32 queueIndex)
{
    vkGetDeviceQueue(device->get(), queueFamilyIndex, queueIndex, &m_queue);

    m_pDevice                   = device;
    
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

        wait();

        m_queue = VK_NULL_HANDLE;

    }
}


ErrType VulkanQueue::submit(const QueueSubmit* payload)
{
    VkSubmitInfo submitIf = { };
    VkCommandBuffer pBuffers[8];
    
    for (U32 bufferCount = 0; bufferCount < payload->numCommandLists; ++bufferCount) {
        VulkanCommandList* pVc      = static_cast<VulkanCommandList*>(payload->pCommandLists[bufferCount]);
        VkCommandBuffer cmdBuffer   = pVc->get();
        pBuffers[bufferCount]       = cmdBuffer;
    }

    submitIf.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitIf.commandBufferCount     = payload->numCommandLists;
    submitIf.pCommandBuffers        = pBuffers;
    submitIf.waitSemaphoreCount     = 0;
    submitIf.signalSemaphoreCount   = 0;
    submitIf.pWaitDstStageMask      = nullptr;

    vkQueueSubmit(m_queue, 1, &submitIf, VK_NULL_HANDLE);
    
    return REC_RESULT_OK;
}
} // Recluse