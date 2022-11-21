//

#include "VulkanResource.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanCommons.hpp"

#include "Recluse/Messaging.hpp"


namespace Recluse {

ErrType VulkanResource::initialize(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState)
{
    VkMemoryRequirements memoryRequirements = { };
    VulkanAllocator* allocator              = nullptr;
    ErrType result                          = RecluseResult_Ok;
    m_pDevice                               = pDevice;
  
    setCurrentResourceState(initState);

    result = onCreate(pDevice, desc, initState);

    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Unable to create resource object!");

        destroy();

        return result;
    }
    
    result = onGetMemoryRequirements(pDevice, memoryRequirements);

    // allocate our resource.
    if (desc.dimension == ResourceDimension_Buffer) 
    {
        allocator = pDevice->getBufferAllocator(desc.memoryUsage);
    } 
    else 
    {
        allocator = pDevice->getImageAllocator(desc.memoryUsage);
    }

    if (!allocator) 
    {
        R_ERR(R_CHANNEL_VULKAN, "No allocator for the given resource to allocate from!");

        destroy();

        return RecluseResult_NullPtrExcept;
    }

    result = allocator->allocate(&m_memory, memoryRequirements);

    if (result != RecluseResult_Ok) 
    {
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
    ErrType result                          = RecluseResult_Ok;

    onDestroy(m_pDevice);

    if (m_memory.deviceMemory) 
    {
        if (desc.dimension == ResourceDimension_Buffer) 
        { 
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated buffer...");
            
            allocator = m_pDevice->getBufferAllocator(desc.memoryUsage);
        } 
        else 
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated image...");

            allocator = m_pDevice->getImageAllocator(desc.memoryUsage);
        }

        if (!allocator) 
        {
            R_ERR(R_CHANNEL_VULKAN, "No allocator provided for this memory! Can not destroy...");

            result = RecluseResult_Failed;
        } 
        else 
        {
            result = allocator->free(&m_memory);
        }

        if (result != RecluseResult_Ok) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Vulkan resource memory failed to free!");
        }

    }
}


ErrType VulkanBuffer::onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState) 
{
    ErrType result                  = RecluseResult_Ok;
    ResourceUsageFlags usageFlags   = desc.usage;
    VkResult vulkanResult           = VK_SUCCESS;

    VkBufferCreateInfo info = { };
    info.sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    info.size           = desc.width;
    info.usage          = 0;

    if (usageFlags & ResourceUsage_VertexBuffer)        info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (usageFlags & ResourceUsage_IndexBuffer)         info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (usageFlags & ResourceUsage_UnorderedAccess)     info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usageFlags & ResourceUsage_TransferDestination) info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (usageFlags & ResourceUsage_TransferSource)      info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usageFlags & ResourceUsage_ConstantBuffer)      info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (usageFlags & ResourceUsage_IndirectBuffer)      info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    
    vulkanResult = vkCreateBuffer(pDevice->get(), &info, nullptr, &m_buffer);

    if (vulkanResult != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan buffer!");
        
        result = RecluseResult_Failed;
    }

    return result;
}


ErrType VulkanBuffer::onDestroy(VulkanDevice* pDevice)
{
    ErrType result = RecluseResult_Ok;

    if (m_buffer) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying buffer...");

        vkDestroyBuffer(pDevice->get(), m_buffer, nullptr);
        
        m_buffer = VK_NULL_HANDLE;
    }
    
    return result;
}


VkFormatFeatureFlags VulkanImage::loadFormatFeatures(VkImageCreateInfo& info, ResourceUsageFlags usage) const
{
    VkFormatFeatureFlags featureFlags = 0;

    if (usage & ResourceUsage_RenderTarget) 
    {
        info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        featureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    }

    if (usage & ResourceUsage_ShaderResource) 
    { 
        info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        featureFlags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    }

    if (usage & ResourceUsage_DepthStencil) 
    { 
        info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        featureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if (usage & ResourceUsage_TransferDestination) 
    { 
        info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        featureFlags |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    }

    if (usage & ResourceUsage_TransferSource) 
    { 
        info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        featureFlags |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    }

    if (usage & ResourceUsage_UnorderedAccess) 
    {
        info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        featureFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    }

    return featureFlags;
}


ErrType VulkanImage::onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState)
{
    ErrType result                      = RecluseResult_Ok;
    ResourceUsageFlags usage            = desc.usage;
    VkResult vulkanResult               = VK_SUCCESS;
    VkFormat format                     = Vulkan::getVulkanFormat(desc.format);
    VkFormatProperties property         = pDevice->getAdapter()->getFormatProperties(format);
    VkImageTiling tiling                = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags featureFlags   = 0;
    
    VkImageCreateInfo info  = { };
    info.sType              = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.arrayLayers        = desc.arrayLevels;
    info.extent.width       = desc.width;   
    info.extent.height      = desc.height;
    info.extent.depth       = desc.depth;
    info.initialLayout      = Vulkan::getVulkanImageLayout(initState);
    info.mipLevels          = desc.mipLevels;
    info.imageType          = VK_IMAGE_TYPE_2D;         
    info.tiling             = tiling;
    info.samples            = Vulkan::getSamples(desc.samples);
    info.format             = format;

    switch (desc.dimension) 
    {
        case ResourceDimension_1d: info.imageType = VK_IMAGE_TYPE_1D; break;
        case ResourceDimension_2d: info.imageType = VK_IMAGE_TYPE_2D; break;
        case ResourceDimension_3d: info.imageType = VK_IMAGE_TYPE_3D; break;
        default: break;
    }
    
    featureFlags = loadFormatFeatures(info, desc.usage);

    if (property.optimalTilingFeatures & featureFlags) 
    {
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
    } 
    else if (property.linearTilingFeatures & featureFlags) 
    {
        info.tiling = VK_IMAGE_TILING_LINEAR;
    } 
    else 
    {
        R_ERR(R_CHANNEL_VULKAN, "Could not find a proper tiling scheme for the given format!");
    }

    switch (info.samples) 
    {
        case VK_SAMPLE_COUNT_1_BIT: break;
        default: info.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; break;
    }

    vulkanResult = vkCreateImage(pDevice->get(), &info, nullptr, &m_image);

    if (vulkanResult != VK_SUCCESS)
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan image!");

        result = RecluseResult_Failed;
    }
    
    m_currentLayout = info.initialLayout;

    return result;
}


ErrType VulkanImage::onDestroy(VulkanDevice* pDevice)
{
    ErrType result = RecluseResult_Ok;

    if (m_image) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying image...");

        vkDestroyImage(pDevice->get(), m_image, nullptr);
            
        m_image = VK_NULL_HANDLE;
    }
    
    return result;
}


ErrType VulkanBuffer::onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements)
{
    ErrType result = RecluseResult_Ok;
    
    vkGetBufferMemoryRequirements(pDevice->get(), m_buffer, &memRequirements);
    
    return result;
}


ErrType VulkanImage::onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements)
{
    vkGetImageMemoryRequirements(pDevice->get(), m_image, &memRequirements);
    
    return RecluseResult_Ok;
}


ErrType VulkanBuffer::onBind(VulkanDevice* pDevice)
{
    const VulkanMemory& memory = getMemory();

    VkResult result = vkBindBufferMemory(pDevice->get(), m_buffer, memory.deviceMemory, memory.offsetBytes);
    
    if (result != VK_SUCCESS) 
    {    
        return RecluseResult_Failed;
    }
    
    return RecluseResult_Ok;
}


ErrType VulkanImage::onBind(VulkanDevice* pDevice)
{
    const VulkanMemory& memory = getMemory();
    
    VkResult result = vkBindImageMemory(pDevice->get(), m_image, memory.deviceMemory, memory.offsetBytes);

    if (result != VK_SUCCESS) 
    {
        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}


ErrType VulkanResource::map(void** ptr, MapRange* pReadRange)
{
    ErrType result              = RecluseResult_Ok;
    VkResult vr                 = VK_SUCCESS;

    VkDeviceSize offsetBytes    = m_memory.offsetBytes;
    VkDeviceSize sizeBytes      = m_memory.sizeBytes;
    
    if (pReadRange) 
    {
        offsetBytes += pReadRange->offsetBytes;
        sizeBytes    = pReadRange->sizeBytes;
    } 

    *ptr = (void*)((PtrType)m_memory.baseAddr + offsetBytes);

    if (vr != VK_SUCCESS) 
    {    
        result = RecluseResult_Failed;
    }

    return result;
}


ErrType VulkanResource::unmap(MapRange* pWriteRange)
{
    const GraphicsResourceDescription& desc = getDesc();
    VkMappedMemoryRange mappedRange         = { };
    ErrType result                          = RecluseResult_Ok;
    VkResult vr                             = VK_SUCCESS;

    VkDeviceMemory deviceMemory             = m_memory.deviceMemory;
    VkDeviceSize offsetBytes                = m_memory.offsetBytes;
    VkDeviceSize sizeBytes                  = VK_WHOLE_SIZE;

    if (pWriteRange) 
    {
        offsetBytes += pWriteRange->offsetBytes;
        sizeBytes    = pWriteRange->sizeBytes;
    }

    mappedRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory  = deviceMemory;
    mappedRange.offset  = offsetBytes;
    mappedRange.size    = sizeBytes;

    if 
        (
            desc.memoryUsage == ResourceMemoryUsage_CpuOnly 
            || desc.memoryUsage == ResourceMemoryUsage_CpuToGpu
        ) 
    {
        m_pDevice->pushFlushMemoryRange(mappedRange);
    } 
    else 
    {
        m_pDevice->pushInvalidateMemoryRange(mappedRange);
    }

    return result;
}


VkImageMemoryBarrier VulkanImage::transition(ResourceState dstState, VkImageSubresourceRange& range)
{
    const GraphicsResourceDescription& description  = getDesc();
    VkImageMemoryBarrier barrier                    = { };

    VkImageLayout dstImageLayout            = Vulkan::getVulkanImageLayout(dstState);
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = m_currentLayout;
    barrier.newLayout                       = dstImageLayout;
    barrier.image                           = m_image;
    barrier.dstAccessMask                   = Vulkan::getDesiredResourceStateAccessMask(dstState) 
                                            | Vulkan::getDesiredHostMemoryUsageAccess(description.memoryUsage);
    barrier.srcAccessMask                   = getCurrentAccessMask();
    barrier.subresourceRange                = range;

    m_currentLayout     = dstImageLayout;
    
    setCurrentResourceState(dstState);
    setCurrentAccessMask(barrier.dstAccessMask);

    return barrier;
}


VkBufferMemoryBarrier VulkanBuffer::transition(ResourceState dstState)
{
    const VulkanMemory& allocation                  = getMemory();
    const GraphicsResourceDescription& description  = getDesc();
    VkBufferMemoryBarrier barrier                   = { };

    barrier.sType           = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.buffer          = m_buffer;
    barrier.offset          = 0; // This is relative to the base offset when called by bindBufferMemory, so we don't need to include another offset.
    barrier.size            = allocation.sizeBytes;
    barrier.srcAccessMask   = getCurrentAccessMask();
    barrier.dstAccessMask   = Vulkan::getDesiredResourceStateAccessMask(dstState) 
                            | Vulkan::getDesiredHostMemoryUsageAccess(description.memoryUsage);

    
    setCurrentResourceState(dstState);
    setCurrentAccessMask(barrier.dstAccessMask);

    return barrier;
}
} // Recluse