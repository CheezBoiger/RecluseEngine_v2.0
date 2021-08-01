//
#include "VulkanDescriptorManager.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "Recluse/Messaging.hpp"

#include <array>
#include <math.h>


namespace Recluse {


void VulkanDescriptorManager::initialize(VulkanDevice* pDevice)
{
    VkDevice device                 = pDevice->get();
    VkResult result                 = VK_SUCCESS;
    VkDescriptorPoolCreateInfo ci   = { };
    std::array<VkDescriptorPoolSize, 6> poolSizes;

    poolSizes[0].type               = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount    = 512;
    poolSizes[1].type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[1].descriptorCount    = UINT16_MAX;
    poolSizes[2].type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount    = UINT16_MAX;
    poolSizes[3].type               = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[3].descriptorCount    = UINT16_MAX;
    poolSizes[4].type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[4].descriptorCount    = UINT16_MAX;
    poolSizes[5].type               = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[5].descriptorCount    = UINT16_MAX;

    ci.sType                        = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.pPoolSizes                   = poolSizes.data();
    ci.poolSizeCount                = (U32)poolSizes.size();
    ci.maxSets                      = UINT16_MAX;
    ci.flags                        = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    result = vkCreateDescriptorPool(device, &ci, nullptr, &m_pool);

    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan descriptor pool!");
    
    }

}


void VulkanDescriptorManager::destroy(VulkanDevice* pDevice)
{
    VkDevice device = pDevice->get();

    if (m_pool) {
    
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan descriptor pool...");
    
        vkDestroyDescriptorPool(device, m_pool, nullptr);
        m_pool = VK_NULL_HANDLE;
    
    }
}
} // Recluse