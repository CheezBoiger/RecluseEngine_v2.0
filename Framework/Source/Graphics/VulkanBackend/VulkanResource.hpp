// 
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "VulkanCommons.hpp"
#include "VulkanAllocator.hpp"

namespace Recluse {
namespace Vulkan {

class VulkanDevice;
class VulkanPrimaryCommandList;

// Vulkan resource inherits from GraphicsResource, which is what will
// be used by the engine API.
class VulkanResource : public GraphicsResource, public VulkanGraphicsObject
{
public:

    VulkanResource(Bool isBuffer, VkAccessFlags currentAccessMask = VK_ACCESS_MEMORY_READ_BIT)
        : GraphicsResource() 
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
    void                release(Bool immediate = false);

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
    ResourceMemoryUsage getMemoryUsage() const { return m_memoryUsage; }
    ResourceDimension   getDimension() const { return m_dimension; }

    //GraphicsAPI getApi() const override { return VulkanGraphicsObject::getApi(); }
protected:
    void                setCurrentAccessMask(VkAccessFlags flags) { m_currentAccessMask = flags; }
    VkAccessFlags       getCurrentAccessMask() const { return m_currentAccessMask; }
    virtual void        performInitialLayout(VulkanDevice* pDevice, ResourceState initState) { }
    virtual void        initializeMetadata(const GraphicsResourceDescription& description);
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
    
    virtual ResultCode  onBind(VulkanDevice* pDevice, const GraphicsResourceDescription& description) { return RecluseResult_NoImpl; }
    virtual ResultCode  onRelease(VulkanDevice* pDevice) { return RecluseResult_NoImpl; }

    VulkanMemory        m_memory;
    VulkanDevice*       m_pDevice;
    VkAccessFlags       m_currentAccessMask;
    VkDeviceSize        m_alignmentRequirement;
    Bool                m_isBuffer;
    ResourceId          m_id;
    ResourceMemoryUsage m_memoryUsage;
    ResourceDimension   m_dimension;

    std::map<Hash64, ResourceViewId> m_resourceIds; 
};


class VulkanBuffer : public VulkanResource 
{
public:

    VulkanBuffer(const GraphicsResourceDescription& desc) 
        : VulkanResource(true)
        , m_buffer(VK_NULL_HANDLE)
        , m_bufferSizeBytes(0u) { }

    VkBuffer                get() const { return m_buffer; }
    VkBufferMemoryBarrier   transition(ResourceState dstState);
    U32                     getBufferSizeBytes() const { return m_bufferSizeBytes; }

private:
    void            performInitialLayout(VulkanDevice* pDevice, ResourceState initState) override;
    ResultCode      onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState) override; 
    ResultCode      onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;
    ResultCode      onBind(VulkanDevice* pDevice, const GraphicsResourceDescription& description) override;
    ResultCode      onRelease(VulkanDevice* pDevice) override;    
    VkBuffer        m_buffer;
    U32             m_bufferSizeBytes;
};


class VulkanImage : public VulkanResource 
{
public:

    VulkanImage(VkImage image = VK_NULL_HANDLE, 
            VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED)
        : VulkanResource(false)
        , m_currentLayout(currentLayout)
        , m_image(image) { }

    // Convenient get.
    //
    VkImage get() const { return m_image; }

    // Get the current layout of this image.
    //
    VkImageLayout           getCurrentLayout() const { return m_currentLayout; }

    void                    overrideCurrentLayout(VkImageLayout layout) { m_currentLayout = layout; }
    void                    initializeMetadata(const GraphicsResourceDescription& description) override;

    // Create memory barrier object to transition the layout of this image to the 
    // specified destination layout.
    // 
    VkImageMemoryBarrier    transition(ResourceState dstState, VkImageSubresourceRange& range);
    VkImageSubresourceRange makeSubresourceRange(ResourceState dstState, U16 baseMip = 0, U16 mipCount = 0, U16 baseLayer = 0, U16 layerCount = 0);
    U32                     getMipLevels() const { return m_mipLevels; }
    U32                     getDepthOrArraySize() const { return m_depthOrArraySize; }
    U32                     getWidth() const { return m_width; }
    U32                     getHeight() const { return m_height; }
    VkFormat                getFormat() const { return m_format; }
    ResourceDimension       getDimension() const { return m_dimension; }

private:
    void                    performInitialLayout(VulkanDevice* pDevice, ResourceState initState) override;
    ResultCode              onCreate(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState) override; 
    ResultCode              onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;
    ResultCode              onBind(VulkanDevice* pDevice, const GraphicsResourceDescription& description) override;
    ResultCode              onRelease(VulkanDevice* pDevice) override;   
    VkFormatFeatureFlags    loadFormatFeatures(VkImageCreateInfo& info, ResourceUsageFlags usage) const;

    VkImage             m_image;
    VkImageLayout       m_currentLayout;
    U16                 m_depthOrArraySize;
    U16                 m_mipLevels;
    U32                 m_width;
    U32                 m_height;
    VkFormat            m_format;
    ResourceDimension   m_dimension;
};


namespace Resources {

VulkanResource*     makeResource(VulkanDevice* pDevice, const GraphicsResourceDescription& desc, ResourceState initState);
ResultCode          releaseResource(VulkanDevice* pDevice, ResourceId id, Bool immediate);
VulkanResource*     obtainResource(ResourceId id);
} // Resources
} // Vulkan
} // Recluse