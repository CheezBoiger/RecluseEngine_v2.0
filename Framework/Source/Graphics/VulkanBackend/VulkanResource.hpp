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

    VulkanResource(const GraphicsResourceDescription& desc, Bool isBuffer, VkAccessFlags currentAccessMask = VK_ACCESS_MEMORY_READ_BIT)
        : GraphicsResource(desc) 
        , m_memory({})
        , m_isBuffer(isBuffer)
        , m_currentAccessMask(currentAccessMask)
        , m_pDevice(nullptr)
        , m_id(~0) { m_memory.deviceMemory = VK_NULL_HANDLE; }

    virtual ~VulkanResource() { }
    
    ResultCode          initialize(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState);

    // Destroy native handle that is managed by this resource object.
    // Must be called before deleting this object.
    //
    void                release();

    // Simply releases views associated with this resource.
    void                releaseViews();

    // Vulkan memory handle, normally assigned by the Vulkan Allocator.
    //
    const VulkanMemory& getMemory() const { return m_memory; }

    ResultCode          map(void** ptr, MapRange* pReadRange) override;
    ResultCode          unmap(MapRange* pWrittenRange) override;

    // Required alignment needed for writing onto this resource.
    VkDeviceSize        getAlignmentRequirement() const { return m_alignmentRequirement; }

    // Get the vulkan device that was used to allocate this resource.
    //
    VulkanDevice*       getDevice() const { return m_pDevice; }

    // Check if the vulkan resource is a buffer. Otherwise, will be an image.
    inline Bool         isBuffer() const { return m_isBuffer; }
    ResourceId          getId() const override { return m_id; }

    ResourceViewId      asView(const ResourceViewDescription& description) override;
    void                setDevice(VulkanDevice* pDevice) { m_pDevice = pDevice; }

    //GraphicsAPI getApi() const override { return VulkanGraphicsObject::getApi(); }
protected:
    void                setCurrentAccessMask(VkAccessFlags flags) { m_currentAccessMask = flags; }
    VkAccessFlags       getCurrentAccessMask() const { return m_currentAccessMask; }

private:
    static ResourceId   kResourceCreationCounter;
    static MutexGuard   kResourceCreationMutex;

public:
    void generateId() override 
    {
        ScopedLock _(kResourceCreationMutex);
        m_id = kResourceCreationCounter++;
    }

private:
    virtual ResultCode  onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState)
        { return RecluseResult_NoImpl; }

    virtual ResultCode  onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) 
        { return RecluseResult_NoImpl; }
    
    virtual ResultCode  onBind(VulkanDevice* pDevice) { return RecluseResult_NoImpl; }
    virtual ResultCode  onRelease(VulkanDevice* pDevice) { return RecluseResult_NoImpl; }

    VulkanMemory        m_memory;
    VulkanDevice*       m_pDevice;
    VkAccessFlags       m_currentAccessMask;
    VkDeviceSize        m_alignmentRequirement;
    Bool                m_isBuffer;
    ResourceId          m_id;

    std::map<Hash64, ResourceViewId> m_resourceIds; 
};


class VulkanBuffer : public VulkanResource 
{
public:

    VulkanBuffer(const GraphicsResourceDescription& desc) 
        : VulkanResource(desc, true)
        , m_buffer(VK_NULL_HANDLE) { }

    VkBuffer                get() const { return m_buffer; }
    VkBufferMemoryBarrier   transition(ResourceState dstState);

private:
    ResultCode      onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState) override; 
    ResultCode      onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;
    ResultCode      onBind(VulkanDevice* pDevice) override;
    ResultCode      onRelease(VulkanDevice* pDevice) override;    
    VkBuffer        m_buffer;
};


class VulkanImage : public VulkanResource 
{
public:

    VulkanImage(const GraphicsResourceDescription& desc, 
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
    VkImageMemoryBarrier    transition(ResourceState dstState, VkImageSubresourceRange& range);
    VkImageSubresourceRange makeSubresourceRange(ResourceState dstState);

private:
    void                    performInitialLayout(VulkanDevice* pDevice, ResourceState initState);
    ResultCode              onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState) override; 
    ResultCode              onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;
    ResultCode              onBind(VulkanDevice* pDevice) override;
    ResultCode              onRelease(VulkanDevice* pDevice) override;   
    VkFormatFeatureFlags    loadFormatFeatures(VkImageCreateInfo& info, ResourceUsageFlags usage) const;

    VkImage             m_image;
    VkImageLayout       m_currentLayout;
};


namespace Resources {

VulkanResource*     makeResource(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState);
ResultCode          releaseResource(VulkanDevice* pDevice, ResourceId id);
VulkanResource*     obtainResource(ResourceId id);
} // Resources
} // Recluse