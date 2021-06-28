//

#include "VulkanResource.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"

#include "Core/Messaging.hpp"

namespace Vulkan {

VkFormat getVulkanFormat(Recluse::ResourceFormat format)
{
    switch (format) {
        case Recluse::RESOURCE_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case Recluse::RESOURCE_FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case Recluse::RESOURCE_FORMAT_R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Recluse::RESOURCE_FORMAT_R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case Recluse::RESOURCE_FORMAT_D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
        case Recluse::RESOURCE_FORMAT_R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
        case Recluse::RESOURCE_FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Recluse::RESOURCE_FORMAT_R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
        default: return VK_FORMAT_UNDEFINED;
    }
}
} // Vulkan

namespace Recluse {

ErrType VulkanResource::initialize(VulkanDevice* pDevice, GraphicsResourceDescription& desc)
{
    VkMemoryRequirements memoryRequirements = { };
    VulkanAllocator* allocator              = nullptr;
    ErrType result                          = REC_RESULT_OK;
    m_pDevice                               = pDevice;

    result = onCreate(pDevice, desc);

    if (result != REC_RESULT_OK) {
    
        R_ERR(R_CHANNEL_VULKAN, "Unable to create resource object!");

        destroy();

        return result;
    
    }
    
    result = onGetMemoryRequirements(pDevice, memoryRequirements);

    // allocate our resource.
    allocator = pDevice->getAllocator(desc.memoryUsage);
   
    result = allocator->allocate(&m_memory, memoryRequirements);

    if (result != REC_RESULT_OK) {
    
        R_ERR(R_CHANNEL_VULKAN, "Unable to allocate memory for resource!");

        destroy();

        return result;
    
    }

    result = onBind(pDevice);

    return result;
}


void VulkanResource::destroy()
{
    VulkanAllocator* allocator              = nullptr;
    const GraphicsResourceDescription& desc = getDesc();

    onDestroy(m_pDevice);

    if (m_memory.deviceMemory) {

        allocator = m_pDevice->getAllocator(desc.memoryUsage);
        allocator->free(&m_memory);

    }
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


ErrType VulkanBuffer::onDestroy(VulkanDevice* pDevice)
{
    ErrType result = REC_RESULT_OK;

    if (m_buffer) {

        R_DEBUG(R_CHANNEL_VULKAN, "Destroying buffer...");

        vkDestroyBuffer(pDevice->get(), m_buffer, nullptr);
        
        m_buffer = VK_NULL_HANDLE;
    
    }
    
    return result;
}


ErrType VulkanImage::onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc)
{
    ErrType result              = REC_RESULT_OK;
    ResourceUsageFlags usage    = desc.usage;
    VkResult vulkanResult       = VK_SUCCESS;
    
    VkImageCreateInfo info = { };
    info.sType              = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.arrayLayers        = desc.arrayLevels;
    info.extent.width       = desc.width;   
    info.extent.height      = desc.height;
    info.extent.depth       = desc.depth;
    info.initialLayout      = VK_IMAGE_LAYOUT_PREINITIALIZED;
    info.mipLevels          = desc.mipLevels;
    info.imageType          = VK_IMAGE_TYPE_2D;         
    info.tiling             = VK_IMAGE_TILING_OPTIMAL;
    info.samples            = Vulkan::getSamples(desc.samples);
    info.format             = Vulkan::getVulkanFormat(desc.format);

    switch (desc.dimension) {
    
        case RESOURCE_DIMENSION_1D: info.imageType = VK_IMAGE_TYPE_1D; break;
        case RESOURCE_DIMENSION_2D: info.imageType = VK_IMAGE_TYPE_2D; break;
        case RESOURCE_DIMENSION_3D: info.imageType = VK_IMAGE_TYPE_3D; break;
        default: break;
    }
    
    if (usage & RESOURCE_USAGE_RENDER_TARGET) info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (usage & RESOURCE_USAGE_SHADER_RESOURCE) info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (usage & RESOURCE_USAGE_DEPTH_STENCIL) info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (usage & RESOURCE_USAGE_TRANSFER_DESTINATION) info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (usage & RESOURCE_USAGE_TRANSFER_SOURCE) info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    switch (info.samples) {
        case VK_SAMPLE_COUNT_1_BIT: break;
        default: info.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; break;
    }

    vulkanResult = vkCreateImage(pDevice->get(), &info, nullptr, &m_image);

    if (vulkanResult != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan image!");

        result = REC_RESULT_FAILED;

    }
    
    m_currentLayout = info.initialLayout;

    return result;
}


ErrType VulkanImage::onDestroy(VulkanDevice* pDevice)
{
    ErrType result = REC_RESULT_OK;

    if (m_image) {
    
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying image...");

        vkDestroyImage(pDevice->get(), m_image, nullptr);
            
        m_image = VK_NULL_HANDLE;
    
    }
    
    return result;
}


ErrType VulkanBuffer::onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements)
{
    ErrType result = REC_RESULT_OK;
    
    vkGetBufferMemoryRequirements(pDevice->get(), m_buffer, &memRequirements);
    
    return result;
}


ErrType VulkanImage::onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements)
{
    vkGetImageMemoryRequirements(pDevice->get(), m_image, &memRequirements);
    
    return REC_RESULT_OK;
}


ErrType VulkanBuffer::onBind(VulkanDevice* pDevice)
{
    const VulkanMemory& memory = getMemory();

    VkResult result = vkBindBufferMemory(pDevice->get(), m_buffer, memory.deviceMemory, memory.offsetBytes);
    
    if (result != VK_SUCCESS) {
        
        return REC_RESULT_FAILED;
    
    }
    
    return REC_RESULT_OK;
}


ErrType VulkanImage::onBind(VulkanDevice* pDevice)
{
    const VulkanMemory& memory = getMemory();
    
    VkResult result = vkBindImageMemory(pDevice->get(), m_image, memory.deviceMemory, memory.offsetBytes);

    if (result != VK_SUCCESS) {
    
        return REC_RESULT_FAILED;
    
    }

    return REC_RESULT_OK;
}


ErrType VulkanResource::map(void** ptr, MapRange* pReadRange)
{
    ErrType result              = REC_RESULT_OK;
    VkResult vr                 = VK_SUCCESS;

    VkDeviceSize offsetBytes    = m_memory.offsetBytes;
    VkDeviceSize sizeBytes      = m_memory.sizeBytes;
    
    if (pReadRange) {

        offsetBytes = pReadRange->offsetBytes;
        sizeBytes   = pReadRange->sizeBytes;

    } 

    *ptr = (void*)((PtrType)m_memory.baseAddr + offsetBytes);

    if (vr != VK_SUCCESS) {
        
        result = REC_RESULT_FAILED;
    
    }

    return result;
}


ErrType VulkanResource::unmap(MapRange* pWriteRange)
{
    ErrType result  = REC_RESULT_OK;
    VkResult vr     = VK_SUCCESS;

    VkDeviceSize offsetBytes    = m_memory.offsetBytes;
    VkDeviceSize sizeBytes      = m_memory.sizeBytes;

    if (pWriteRange) {
    
        offsetBytes = pWriteRange->offsetBytes;
        sizeBytes   = pWriteRange->sizeBytes;
    
    }

    

    return result;
}
} // Recluse