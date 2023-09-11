//

#include "VulkanResource.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanCommons.hpp"
#include "VulkanQueue.hpp"
#include "Recluse/Messaging.hpp"


namespace Recluse {
namespace Resources {

std::unordered_map<ResourceId, VulkanResource*> g_resourcesMap;

VulkanResource* makeResource(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState)
{
    R_ASSERT_FORMAT(desc.width > 0 && desc.height > 0 && desc.depthOrArraySize > 0 && desc.mipLevels > 0, "Description width/height/arraySize/mipLevels should at least be 1 or greater!");
    if (desc.dimension == ResourceDimension_Buffer)
    {
        VulkanBuffer* pBuffer   = new VulkanBuffer(desc);
        pBuffer->initialize(pDevice, desc, initState);
        pBuffer->generateId();
        ResourceId id           = pBuffer->getId();
        g_resourcesMap[id]      = pBuffer;
        return g_resourcesMap[id];
    }
    else
    {
        VulkanImage* pImage = new VulkanImage();
        pImage->initialize(pDevice, desc, initState);
        pImage->generateId();
        ResourceId id       = pImage->getId();
        g_resourcesMap[id]  = pImage;
        return g_resourcesMap[id];
    }

    return nullptr;
}


ResultCode releaseResource(VulkanDevice* pDevice, ResourceId id)
{
    auto& iter = g_resourcesMap.find(id);
    if (iter != g_resourcesMap.end())
    {
        iter->second->release();
        g_resourcesMap.erase(iter);
        return RecluseResult_Ok;
    }

    return RecluseResult_NotFound;
}


VulkanResource* obtainResource(ResourceId id)
{
    auto& iter = g_resourcesMap.find(id);
    if (iter == g_resourcesMap.end())
        return nullptr;
    return iter->second;
}
} // Resources

ResourceId VulkanResource::kResourceCreationCounter = 0;
MutexGuard VulkanResource::kResourceCreationMutex        = MutexGuard("VulkanCreationMutex");

ResultCode VulkanResource::initialize(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState)
{
    VkMemoryRequirements memoryRequirements = { };
    VulkanAllocationManager* allocator      = pDevice->getAllocationManager();
    ResultCode result                          = RecluseResult_Ok;
    m_pDevice                               = pDevice;
    m_memoryUsage                           = desc.memoryUsage;
    m_dimension                             = desc.dimension;
    setCurrentResourceState(initState);

    result = onCreate(pDevice, desc, initState);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Unable to create resource object!");

        release();

        return result;
    }
    
    result = onGetMemoryRequirements(pDevice, memoryRequirements);

    // allocate our resource.
    if (desc.dimension == ResourceDimension_Buffer) 
    {
        result = allocator->allocateBuffer(&m_memory, desc.memoryUsage, memoryRequirements);
    } 
    else 
    {
        result = allocator->allocateImage(&m_memory, desc.memoryUsage, memoryRequirements, VK_IMAGE_TILING_OPTIMAL);
    }

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Unable to allocate memory for resource!");

        release();

        return result;
    }

    m_alignmentRequirement = memoryRequirements.alignment;

    result = onBind(pDevice, desc);

    return result;
}


void VulkanResource::release()
{
    VulkanAllocationManager* allocator      = m_pDevice->getAllocationManager();
    ResultCode result                          = RecluseResult_Ok;

    onRelease(m_pDevice);

    releaseViews();

    if (m_memory.deviceMemory) 
    {
        result = allocator->free(&m_memory);
        if (result != RecluseResult_Ok) 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Vulkan resource memory failed to free!");
        }

    }
}


void VulkanResource::releaseViews()
{
    // Clear all remaining resource views associated.
    for (auto iter : m_resourceIds)
    {
        ResourceViewId id = iter.second;
        ResourceViews::releaseResourceView(m_pDevice, id);
    }

    m_resourceIds.clear();
}


void VulkanResource::initializeMetadata(const GraphicsResourceDescription& description)
{
    m_dimension     = description.dimension;
    m_memoryUsage   = description.memoryUsage;
}


ResultCode VulkanBuffer::onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState) 
{
    ResultCode result                  = RecluseResult_Ok;
    ResourceUsageFlags usageFlags   = desc.usage;
    VkResult vulkanResult           = VK_SUCCESS;
    // Initial size in bytes, this is what the buffer itself would be seen with.
    m_bufferSizeBytes               = desc.width;

    VkBufferCreateInfo info = { };
    info.sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    info.size           = desc.width;
    info.usage          = 0;

    if (usageFlags & ResourceUsage_VertexBuffer)        info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (usageFlags & ResourceUsage_IndexBuffer)         info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (usageFlags & ResourceUsage_UnorderedAccess)     info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usageFlags & ResourceUsage_ShaderResource)      info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usageFlags & ResourceUsage_TransferDestination) info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (usageFlags & ResourceUsage_TransferSource)      info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usageFlags & ResourceUsage_ConstantBuffer)      info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (usageFlags & ResourceUsage_IndirectBuffer)      info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    
    vulkanResult = vkCreateBuffer(pDevice->get(), &info, nullptr, &m_buffer);

    if (vulkanResult != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create vulkan buffer!");
        
        result = RecluseResult_Failed;
    }

    return result;
}


ResultCode VulkanBuffer::onRelease(VulkanDevice* pDevice)
{
    ResultCode result = RecluseResult_Ok;

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


ResultCode VulkanImage::onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState)
{
    ResultCode result                   = RecluseResult_Ok;
    ResourceUsageFlags usage            = desc.usage;
    VkResult vulkanResult               = VK_SUCCESS;
    VkFormat format                     = Vulkan::getVulkanFormat(desc.format);
    VkFormatProperties property         = pDevice->getAdapter()->getFormatProperties(format);
    VkImageTiling tiling                = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags featureFlags   = 0;
    
    VkImageCreateInfo info  = { };
    info.sType              = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.arrayLayers        = (info.imageType != ResourceDimension_3d) ? desc.depthOrArraySize : 1;
    info.extent.width       = desc.width;   
    info.extent.height      = desc.height;
    info.extent.depth       = (info.imageType == ResourceDimension_3d) ? desc.depthOrArraySize : 1;
    info.initialLayout      = VK_IMAGE_LAYOUT_UNDEFINED; // this is a must, since the vulkan spec states it needs to be either UNDEFINED, or PREINITIALIZED.
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
        R_ERROR(R_CHANNEL_VULKAN, "Could not find a proper tiling scheme for the given format!");
    }

    switch (info.samples) 
    {
        case VK_SAMPLE_COUNT_1_BIT: break;
        default: info.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; break;
    }

    vulkanResult = vkCreateImage(pDevice->get(), &info, nullptr, &m_image);

    if (vulkanResult != VK_SUCCESS)
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create vulkan image!");

        result = RecluseResult_Failed;
    }
    
    m_currentLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    m_depthOrArraySize  = static_cast<U16>(desc.depthOrArraySize);
    m_mipLevels         = static_cast<U16>(desc.mipLevels);
    m_width             = desc.width;
    m_height            = desc.height;
    m_depthOrArraySize  = desc.depthOrArraySize;   
    m_format            = format;

    return result;
}


ResultCode VulkanImage::onRelease(VulkanDevice* pDevice)
{
    ResultCode result = RecluseResult_Ok;

    if (m_image) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying image...");

        vkDestroyImage(pDevice->get(), m_image, nullptr);
            
        m_image = VK_NULL_HANDLE;
    }
    
    return result;
}


ResultCode VulkanBuffer::onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements)
{
    ResultCode result = RecluseResult_Ok;
    
    vkGetBufferMemoryRequirements(pDevice->get(), m_buffer, &memRequirements);
    
    return result;
}


ResultCode VulkanImage::onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements)
{
    vkGetImageMemoryRequirements(pDevice->get(), m_image, &memRequirements);
    
    return RecluseResult_Ok;
}


ResultCode VulkanBuffer::onBind(VulkanDevice* pDevice, const GraphicsResourceDescription& description)
{
    const VulkanMemory& memory = getMemory();

    VkResult result = vkBindBufferMemory(pDevice->get(), m_buffer, memory.deviceMemory, memory.offsetBytes);
    
    if (result != VK_SUCCESS) 
    {    
        return RecluseResult_Failed;
    }

    const Bool supportsDebugMarking = pDevice->getAdapter()->getInstance()->supportsDebugMarking();
    const char* debugName           = description.name;
    if (supportsDebugMarking && debugName)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo  = { };
        nameInfo.sType                          = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType                     = VK_OBJECT_TYPE_BUFFER;
        nameInfo.pNext                          = nullptr;
        nameInfo.objectHandle                   = reinterpret_cast<uint64_t>(m_buffer);
        nameInfo.pObjectName                    = debugName;
        VkResult debugResult                    = pfn_vkSetDebugUtilsObjectNameEXT(pDevice->get(), &nameInfo);
        if (debugResult != VK_SUCCESS)
        {
            R_WARN(R_CHANNEL_VULKAN, "Failed to create buffer debug name object.");
        }
    }    
    performInitialLayout(getDevice(), getCurrentResourceState());
    return RecluseResult_Ok;
}


ResultCode VulkanImage::onBind(VulkanDevice* pDevice, const GraphicsResourceDescription& description)
{
    const VulkanMemory& memory = getMemory();
    
    VkResult result = vkBindImageMemory(pDevice->get(), m_image, memory.deviceMemory, memory.offsetBytes);

    if (result != VK_SUCCESS) 
    {
        return RecluseResult_Failed;
    }

    const Bool supportsDebugMarking = pDevice->getAdapter()->getInstance()->supportsDebugMarking();
    const char* debugName           = description.name;
    if (supportsDebugMarking && debugName)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo  = { };
        nameInfo.sType                          = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType                     = VK_OBJECT_TYPE_IMAGE;
        nameInfo.pNext                          = nullptr;
        nameInfo.objectHandle                   = reinterpret_cast<uint64_t>(m_image);
        nameInfo.pObjectName                    = debugName;
        VkResult debugResult                    = pfn_vkSetDebugUtilsObjectNameEXT(pDevice->get(), &nameInfo);
        if (debugResult != VK_SUCCESS)
        {
            R_WARN(R_CHANNEL_VULKAN, "Failed to create image debug name object.");
        }
    }

    // Here after we bind memory, we must transition to initial state.
    performInitialLayout(pDevice, getCurrentResourceState());

    return RecluseResult_Ok;
}


ResultCode VulkanResource::map(void** ptr, MapRange* pReadRange)
{
    ResultCode result              = RecluseResult_Ok;
    VkResult vr                 = VK_SUCCESS;

    VkDeviceSize offsetBytes    = m_memory.offsetBytes;
    VkDeviceSize sizeBytes      = m_memory.sizeBytes;
    
    if (pReadRange) 
    {
        offsetBytes += align(pReadRange->offsetBytes, VulkanAdapter::obtainMinMemoryMapAlignment(m_pDevice));
        sizeBytes    = pReadRange->sizeBytes;
    } 

    *ptr = (void*)((UPtr)m_memory.baseAddr + offsetBytes);

    if (vr != VK_SUCCESS) 
    {    
        result = RecluseResult_Failed;
    }

    return result;
}


ResultCode VulkanResource::unmap(MapRange* pWriteRange)
{
    VkMappedMemoryRange mappedRange         = { };
    ResultCode result                          = RecluseResult_Ok;
    VkResult vr                             = VK_SUCCESS;

    VkDeviceMemory deviceMemory             = m_memory.deviceMemory;
    VkDeviceSize offsetBytes                = m_memory.offsetBytes;
    VkDeviceSize sizeBytes                  = VK_WHOLE_SIZE;

    if (pWriteRange) 
    {
        offsetBytes += align(pWriteRange->offsetBytes, VulkanAdapter::obtainMinMemoryMapAlignment(m_pDevice));
        sizeBytes    = pWriteRange->sizeBytes;
    }

    mappedRange.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory  = deviceMemory;
    mappedRange.offset  = offsetBytes;
    mappedRange.size    = sizeBytes;

    const ResourceMemoryUsage memUsage = getMemoryUsage();
    if 
        (
            memUsage == ResourceMemoryUsage_CpuOnly 
            || memUsage == ResourceMemoryUsage_CpuToGpu
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
    VkImageMemoryBarrier barrier                    = { };

    VkImageLayout dstImageLayout            = Vulkan::getVulkanImageLayout(dstState);
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = m_currentLayout;
    barrier.newLayout                       = dstImageLayout;
    barrier.image                           = m_image;
    barrier.dstAccessMask                   = Vulkan::getDesiredResourceStateAccessMask(dstState) 
                                            | Vulkan::getDesiredHostMemoryUsageAccess(getMemoryUsage());
    barrier.srcAccessMask                   = getCurrentAccessMask();
    barrier.subresourceRange                = range;

    m_currentLayout     = dstImageLayout;
    
    setCurrentResourceState(dstState);
    setCurrentAccessMask(barrier.dstAccessMask);

    return barrier;
}


VkImageSubresourceRange VulkanImage::makeSubresourceRange(ResourceState dstState, U16 baseMip, U16 mipCount, U16 baseLayer, U16 layerCount)
{
        U32 baseArrayLayer                  = baseLayer;
        U32 baseMipLevel                    = baseMip;
        U32 arrayLayerCount                 = layerCount;
        U32 mipLevelCount                   = mipCount;

        if (layerCount == 0)
        {
            arrayLayerCount = m_depthOrArraySize - baseArrayLayer;
        }

        if (mipLevelCount == 0)
        {
            mipLevelCount = m_mipLevels - baseMipLevel;
        }

        VkImageSubresourceRange range       = { };
        range.baseArrayLayer                = baseArrayLayer;
        range.baseMipLevel                  = baseMipLevel;
        range.layerCount                    = arrayLayerCount;
        range.levelCount                    = mipLevelCount;
        range.aspectMask                    = VK_IMAGE_ASPECT_COLOR_BIT;
        
        if 
            (
                dstState == ResourceState_DepthStencilReadOnly || 
                dstState == ResourceState_DepthStencilWrite
            )
        {
            range.aspectMask = Vulkan::getDepthStencilAspectFlags(m_format);
        }
    return range;
}


VkBufferMemoryBarrier VulkanBuffer::transition(ResourceState dstState)
{
    VkBufferMemoryBarrier barrier                   = { };

    barrier.sType           = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.buffer          = m_buffer;
    barrier.offset          = 0; // This is relative to the base offset when called by bindBufferMemory, so we don't need to include another offset.
    barrier.size            = getBufferSizeBytes();
    barrier.srcAccessMask   = getCurrentAccessMask();
    barrier.dstAccessMask   = Vulkan::getDesiredResourceStateAccessMask(dstState) 
                            | Vulkan::getDesiredHostMemoryUsageAccess(getMemoryUsage());
    
    setCurrentResourceState(dstState);
    setCurrentAccessMask(barrier.dstAccessMask);

    return barrier;
}


ResourceViewId VulkanResource::asView(const ResourceViewDescription& description)
{
    Hash64 hash = recluseHashFast(&description, sizeof(ResourceViewDescription));
    ResourceViewId viewId = 0;
    auto iter = m_resourceIds.find(hash);
    if (iter == m_resourceIds.end())
    {
        viewId = ResourceViews::makeResourceView(m_pDevice, this, description);
        m_resourceIds.insert(std::make_pair(hash, viewId));
    }
    else
    {
        viewId = iter->second;
    }

    return viewId;
}


void VulkanImage::performInitialLayout(VulkanDevice* pDevice, ResourceState initialState)
{
    VkImageLayout layout = Vulkan::getVulkanImageLayout(initialState);
    VulkanQueue* pQueue = pDevice->getQueue((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    if (layout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        return;
    }

    VkCommandBuffer transitionCmd = pQueue->beginOneTimeCommandBuffer();
    VkImageSubresourceRange range = makeSubresourceRange(initialState);
    VkImageMemoryBarrier barrier = transition(initialState, range);
    vkCmdPipelineBarrier
        (
            transitionCmd, 
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 
            0, nullptr, 0, nullptr, 
            1, &barrier
        );
    pQueue->endAndSubmitOneTimeCommandBuffer(transitionCmd);
}


void VulkanBuffer::performInitialLayout(VulkanDevice* pDevice, ResourceState initialState)
{
    VulkanQueue* pQueue = pDevice->getQueue((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    VkCommandBuffer transitionCmd = pQueue->beginOneTimeCommandBuffer();
    VkBufferMemoryBarrier barrier = transition(initialState);
    vkCmdPipelineBarrier
        (
            transitionCmd, 
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 
            0, nullptr, 
            1, &barrier, 
            0, nullptr
        );
    pQueue->endAndSubmitOneTimeCommandBuffer(transitionCmd);
}


void VulkanImage::initializeMetadata(const GraphicsResourceDescription& description)
{
    VulkanResource::initializeMetadata(description);
    m_depthOrArraySize = description.depthOrArraySize;
    m_width = description.width;
    m_height = description.height;
    m_mipLevels = description.mipLevels;
    m_format = Vulkan::getVulkanFormat(description.format);
}
} // Recluse