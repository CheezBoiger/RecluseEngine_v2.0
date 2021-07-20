//

#include "VulkanResource.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"

#include "Recluse/Messaging.hpp"

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
        case Recluse::RESOURCE_FORMAT_R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Recluse::RESOURCE_FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
        case Recluse::RESOURCE_FORMAT_R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
        case Recluse::RESOURCE_FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
        case Recluse::RESOURCE_FORMAT_R8_UINT: return VK_FORMAT_R8_UINT;
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
    if (desc.dimension == RESOURCE_DIMENSION_BUFFER) {

        allocator = pDevice->getBufferAllocator(desc.memoryUsage);

    } else {

        allocator = pDevice->getImageAllocator(desc.memoryUsage);

    }

    if (!allocator) {

        R_ERR(R_CHANNEL_VULKAN, "No allocator for the given resource to allocate from!");

        destroy();

        return REC_RESULT_NULL_PTR_EXCEPTION;
    }

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
    ErrType result                          = REC_RESULT_OK;

    onDestroy(m_pDevice);

    if (m_memory.deviceMemory) {

        if (desc.dimension == RESOURCE_DIMENSION_BUFFER) { 

            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated buffer...");
            
            allocator = m_pDevice->getBufferAllocator(desc.memoryUsage);

        } else {

            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated image...");

            allocator = m_pDevice->getImageAllocator(desc.memoryUsage);

        }

        if (!allocator) {
    
            R_ERR(R_CHANNEL_VULKAN, "No allocator provided for this memory! Can not destroy...");

            result = REC_RESULT_FAILED;
        
        } else {

            result = allocator->free(&m_memory);

        }

        if (result != REC_RESULT_OK) {
    
            R_ERR(R_CHANNEL_VULKAN, "Vulkan resource memory failed to free!");
    
        }

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


VkFormatFeatureFlags VulkanImage::loadFormatFeatures(VkImageCreateInfo& info, ResourceUsageFlags usage) const
{
    VkFormatFeatureFlags featureFlags = 0;

    if (usage & RESOURCE_USAGE_RENDER_TARGET) {
        info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        featureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    }

    if (usage & RESOURCE_USAGE_SHADER_RESOURCE) { 
        info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        featureFlags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    }

    if (usage & RESOURCE_USAGE_DEPTH_STENCIL) { 
        info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        featureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if (usage & RESOURCE_USAGE_TRANSFER_DESTINATION) { 
        info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        featureFlags |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    }

    if (usage & RESOURCE_USAGE_TRANSFER_SOURCE) { 
        info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        featureFlags |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    }
    if (usage & RESOURCE_USAGE_STORAGE_IMAGE) {
        info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        featureFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    }
    return featureFlags;
}


ErrType VulkanImage::onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc)
{
    ErrType result                      = REC_RESULT_OK;
    ResourceUsageFlags usage            = desc.usage;
    VkResult vulkanResult               = VK_SUCCESS;
    VkFormat format                     = Vulkan::getVulkanFormat(desc.format);
    VkFormatProperties property         = pDevice->getAdapter()->getFormatProperties(format);
    VkImageTiling tiling                = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags featureFlags   = 0;
    
    VkImageCreateInfo info = { };
    info.sType              = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.arrayLayers        = desc.arrayLevels;
    info.extent.width       = desc.width;   
    info.extent.height      = desc.height;
    info.extent.depth       = desc.depth;
    info.initialLayout      = VK_IMAGE_LAYOUT_PREINITIALIZED;
    info.mipLevels          = desc.mipLevels;
    info.imageType          = VK_IMAGE_TYPE_2D;         
    info.tiling             = tiling;
    info.samples            = Vulkan::getSamples(desc.samples);
    info.format             = format;

    switch (desc.dimension) {
    
        case RESOURCE_DIMENSION_1D: info.imageType = VK_IMAGE_TYPE_1D; break;
        case RESOURCE_DIMENSION_2D: info.imageType = VK_IMAGE_TYPE_2D; break;
        case RESOURCE_DIMENSION_3D: info.imageType = VK_IMAGE_TYPE_3D; break;
        default: break;
    }
    
    featureFlags = loadFormatFeatures(info, desc.usage);

    if (property.optimalTilingFeatures & featureFlags) {
    
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
    
    } else if (property.linearTilingFeatures & featureFlags) {
    
        info.tiling = VK_IMAGE_TILING_LINEAR;
    
    } else {
    
        R_ERR(R_CHANNEL_VULKAN, "Could not find a proper tiling scheme for the given format!");
    
    }

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

        offsetBytes += pReadRange->offsetBytes;
        sizeBytes    = pReadRange->sizeBytes;

    } 

    *ptr = (void*)((PtrType)m_memory.baseAddr + offsetBytes);

    if (vr != VK_SUCCESS) {
        
        result = REC_RESULT_FAILED;
    
    }

    return result;
}


ErrType VulkanResource::unmap(MapRange* pWriteRange)
{
    const GraphicsResourceDescription& desc = getDesc();
    VkMappedMemoryRange mappedRange         = { };
    ErrType result                          = REC_RESULT_OK;
    VkResult vr                             = VK_SUCCESS;

    VkDeviceMemory deviceMemory             = m_memory.deviceMemory;
    VkDeviceSize offsetBytes                = m_memory.offsetBytes;
    VkDeviceSize sizeBytes                  = VK_WHOLE_SIZE;

    if (pWriteRange) {
    
        offsetBytes += pWriteRange->offsetBytes;
        sizeBytes    = pWriteRange->sizeBytes;
    
    }

    mappedRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory  = deviceMemory;
    mappedRange.offset  = offsetBytes;
    mappedRange.size    = sizeBytes;

    if (desc.memoryUsage == RESOURCE_MEMORY_USAGE_CPU_ONLY || 
        desc.memoryUsage == RESOURCE_MEMORY_USAGE_CPU_TO_GPU) {

        m_pDevice->pushFlushMemoryRange(mappedRange);

    } else {

        m_pDevice->pushInvalidateMemoryRange(mappedRange);

    }

    return result;
}


VkImageMemoryBarrier VulkanImage::transition(VkImageLayout dstLayout, VkImageSubresourceRange& range)
{
    VkImageMemoryBarrier barrier = { };
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = m_currentLayout;
    barrier.newLayout                       = dstLayout;
    barrier.image                           = m_image;
    barrier.dstAccessMask                   = 0;
    barrier.srcAccessMask                   = m_currentAccessMask;
    barrier.subresourceRange                = range;

    switch (dstLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; 
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; 
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            barrier.dstAccessMask = (VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT); 
            break;
        default: break;
    }

    m_currentLayout     = dstLayout;
    m_currentAccessMask = barrier.dstAccessMask;

    return barrier;
}
} // Recluse