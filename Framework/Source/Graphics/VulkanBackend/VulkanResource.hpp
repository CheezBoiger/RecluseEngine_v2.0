// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "VulkanCommons.hpp"
#include "VulkanAllocator.hpp"

namespace Recluse {


class VulkanDevice;
class VulkanPrimaryCommandList;

// Vulkan resource inherits from GraphicsResource, which is what will
// be used by the engine API.
class VulkanResource : public GraphicsResource, public VulkanGraphicsObject
{
public:

    VulkanResource(GraphicsResourceDescription& desc, Bool isBuffer, VkAccessFlags currentAccessMask = VK_ACCESS_MEMORY_READ_BIT)
        : GraphicsResource(desc) 
        , m_memory({})
        , m_isBuffer(isBuffer)
        , m_currentAccessMask(currentAccessMask)
        , m_pDevice(nullptr) { m_memory.deviceMemory = VK_NULL_HANDLE; }

    virtual ~VulkanResource() { }
    
    ErrType initialize(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState);

    // Destroy native handle that is managed by this resource object.
    // Must be called before deleting this object.
    //
    void destroy();

    // Vulkan memory handle, normally assigned by the Vulkan Allocator.
    //
    const VulkanMemory& getMemory() const { return m_memory; }

    ErrType map(void** ptr, MapRange* pReadRange) override;

    ErrType unmap(MapRange* pWrittenRange) override;

    // Get the vulkan device that was used to allocate this resource.
    //
    VulkanDevice* getDevice() const { return m_pDevice; }

    // Check if the vulkan resource is a buffer. Otherwise, will be an image.
    inline Bool isBuffer() const { return m_isBuffer; }

    //GraphicsAPI getApi() const override { return VulkanGraphicsObject::getApi(); }
protected:
    void setCurrentAccessMask(VkAccessFlags flags) { m_currentAccessMask = flags; }
    VkAccessFlags getCurrentAccessMask() const { return m_currentAccessMask; }

private:
    virtual ErrType onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState)
        { return RecluseResult_NoImpl; }

    virtual ErrType onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) 
        { return RecluseResult_NoImpl; }
    
    virtual ErrType onBind(VulkanDevice* pDevice) { return RecluseResult_NoImpl; }

    virtual ErrType onDestroy(VulkanDevice* pDevice) { return RecluseResult_NoImpl; }

    VulkanMemory m_memory;

    VulkanDevice* m_pDevice;

    VkAccessFlags       m_currentAccessMask;
    Bool m_isBuffer;
};


class VulkanBuffer : public VulkanResource 
{
public:

    VulkanBuffer(GraphicsResourceDescription& desc) 
        : VulkanResource(desc, true)
        , m_buffer(VK_NULL_HANDLE) { }

    VkBuffer get() const { return m_buffer; }

    VkBufferMemoryBarrier transition(ResourceState dstState);

private:
    ErrType onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState) override; 

    ErrType onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;

    ErrType onBind(VulkanDevice* pDevice) override;
    
    ErrType onDestroy(VulkanDevice* pDevice) override;    

    VkBuffer        m_buffer;
};


class VulkanImage : public VulkanResource 
{
public:

    VulkanImage(GraphicsResourceDescription& desc, 
            VkImage image = VK_NULL_HANDLE, 
            VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED)
        : VulkanResource(desc, false)
        , m_currentLayout(currentLayout)
        , m_image(image) { }

    // Convenient get.
    //
    VkImage get() const { return m_image; }

    // Get the current layout of this image.
    //
    VkImageLayout       getCurrentLayout() const { return m_currentLayout; }

    void                overrideCurrentLayout(VkImageLayout layout) { m_currentLayout = layout; }

    // Create memory barrier object to transition the layout of this image to the 
    // specified destination layout.
    // 
    VkImageMemoryBarrier transition(ResourceState dstState, VkImageSubresourceRange& range);

private:
    ErrType             onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc, ResourceState initState) override; 

    ErrType             onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;

    ErrType             onBind(VulkanDevice* pDevice) override;
    
    ErrType             onDestroy(VulkanDevice* pDevice) override;   

    VkFormatFeatureFlags loadFormatFeatures(VkImageCreateInfo& info, ResourceUsageFlags usage) const;

    VkImage             m_image;
    VkImageLayout       m_currentLayout;
};
} // Recluse