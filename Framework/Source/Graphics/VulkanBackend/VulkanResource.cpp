//

#include "VulkanResource.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"

#include "Core/Messaging.hpp"

namespace Recluse {

ErrType VulkanResource::initialize(VulkanDevice* pDevice, GraphicsResourceDescription& desc)
{
    VkMemoryRequirements memoryRequirements = { };
    VulkanAllocator* allocator              = nullptr;

    onCreate(pDevice, desc);
    
    onGetMemoryRequirements(pDevice, memoryRequirements);

    // allocate our resource.
    allocator = pDevice->getAllocator(desc.memoryUsage);
   
    allocator->allocate(&m_memory, memoryRequirements);

    onBind(pDevice);

    return REC_RESULT_OK;
}


void VulkanResource::destroy(VulkanDevice* pDevice)
{
    VulkanAllocator* allocator              = nullptr;
    const GraphicsResourceDescription& desc = getDesc();

    onDestroy(pDevice);

    allocator = pDevice->getAllocator(desc.memoryUsage);
    allocator->free(&m_memory);
}


ErrType VulkanBuffer::onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc) 
{
    ErrType result                  = REC_RESULT_OK;
    ResourceUsageFlags usageFlags   = desc.usage;
    VkResult vulkanResult           = VK_SUCCESS;

    VkBufferCreateInfo info = { };
    info.sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    info.size           = desc.width;
    info.usage          = 0;

    if (usageFlags & RESOURCE_USAGE_VERTEX_BUFFER) info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (usageFlags & RESOURCE_USAGE_INDEX_BUFFER) info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (usageFlags & RESOURCE_USAGE_STORAGE_BUFFER) info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usageFlags & RESOURCE_USAGE_TRANSFER_DESTINATION) info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (usageFlags & RESOURCE_USAGE_TRANSFER_SOURCE) info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usageFlags & RESOURCE_USAGE_CONSTANT_BUFFER) info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (usageFlags & RESOURCE_USAGE_INDIRECT) info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    
    vulkanResult = vkCreateBuffer(pDevice->get(), &info, nullptr, &m_buffer);

    if (vulkanResult != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan buffer!");
        
        result = REC_RESULT_FAILED;
    
    }

    return result;
}
} // Recluse