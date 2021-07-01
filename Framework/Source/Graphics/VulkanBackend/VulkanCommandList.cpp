//
#include "VulkanCommandList.hpp"
#include "VulkanDevice.hpp"

#include "Core/Messaging.hpp"

namespace Recluse {


ErrType VulkanCommandList::initialize(VulkanDevice* pDevice, U32 queueFamilyIndex, 
    VkCommandPool* pools, U32 poolCount)
{
    ErrType result                                  = REC_RESULT_OK;
    VkDevice device                                 = pDevice->get();

    m_buffers.resize(poolCount);
    m_pools.resize(m_buffers.size());

    for (U32 j = 0; j < poolCount; ++j) {

        VkCommandPool pool                    = pools[j];
        VkCommandBufferAllocateInfo allocInfo = { };

        allocInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool         = pool;
        allocInfo.commandBufferCount  = 1;
        allocInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkResult r = vkAllocateCommandBuffers(device, &allocInfo, &m_buffers[j]);

        if (r != VK_SUCCESS) {

            R_ERR(R_CHANNEL_VULKAN, "Failed to allocate command buffer!");

        }
        
        m_pools[j] = pools[j];
    }

    m_pDevice = pDevice;

    return result;
}


void VulkanCommandList::destroy(VulkanDevice* pDevice)
{
    for (U32 i = 0; i < m_buffers.size(); ++i) {
        
        if (m_buffers[i] != VK_NULL_HANDLE) {
        
            vkFreeCommandBuffers(pDevice->get(), m_pools[i], 1, &m_buffers[i]);
            m_buffers[i] = VK_NULL_HANDLE;
        
        }
    
    }
}


void VulkanCommandList::begin()
{
    VkCommandBufferBeginInfo info = { };
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags              = 0;
    info.pInheritanceInfo   = nullptr;
    U32 bufIndex            = m_pDevice->getCurrentBufferIndex();
    VkCommandBuffer buffer  = m_buffers[bufIndex];
    VkResult result = vkBeginCommandBuffer(buffer, &info);

    R_ASSERT(result == VK_SUCCESS);
}


void VulkanCommandList::end()
{
    U32 bufIdx = m_pDevice->getCurrentBufferIndex();
    VkCommandBuffer buffer = m_buffers[bufIdx];
    VkResult result = vkEndCommandBuffer(buffer);
    
    R_ASSERT(result == VK_SUCCESS);
}
} // Recluse