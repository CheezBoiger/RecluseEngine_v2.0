// 
#pragma once

#include "Graphics/Resource.hpp"
#include "VulkanCommons.hpp"
#include "VulkanAllocator.hpp"

namespace Recluse {


class VulkanDevice;
class VulkanCommandList;

class VulkanResource : public GraphicsResource {
public:
    VulkanResource(GraphicsResourceDescription& desc)
        : GraphicsResource(desc) 
        , m_memory({})
        , m_pDevice(nullptr) { m_memory.deviceMemory = VK_NULL_HANDLE; }

    virtual ~VulkanResource() { }
    
    ErrType initialize(VulkanDevice* pDevice, GraphicsResourceDescription& desc);

    void destroy();

    const VulkanMemory& getMemory() const { return m_memory; }

    ErrType map(void** ptr, MapRange* pReadRange) override;

    ErrType unmap(MapRange* pWrittenRange) override;

    VulkanDevice* getDevice() const { return m_pDevice; }

private:
    virtual ErrType onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc)
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) 
        { return REC_RESULT_NOT_IMPLEMENTED; }
    
    virtual ErrType onBind(VulkanDevice* pDevice) { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType onDestroy(VulkanDevice* pDevice) { return REC_RESULT_NOT_IMPLEMENTED; }

    VulkanMemory m_memory;

    VulkanDevice* m_pDevice;
};


class VulkanBuffer : public VulkanResource {
public:
    VulkanBuffer(GraphicsResourceDescription& desc) 
        : VulkanResource(desc)
        , m_buffer(VK_NULL_HANDLE) { }

    VkBuffer get() const { return m_buffer; }

private:
    ErrType onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc) override; 

    ErrType onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;

    ErrType onBind(VulkanDevice* pDevice) override;
    
    ErrType onDestroy(VulkanDevice* pDevice) override;    

    VkBuffer m_buffer;
};


class VulkanImage : public VulkanResource {
public:
    VulkanImage(GraphicsResourceDescription& desc)
        : VulkanResource(desc)
        , m_currentAccessMask(VK_ACCESS_MEMORY_READ_BIT)
        , m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        , m_image(VK_NULL_HANDLE) { }

    VkImage get() const { return m_image; }

    VkImageLayout getCurrentLayout() const { return m_currentLayout; }

    VkImageMemoryBarrier transition(VkImageLayout dstLayout, VkImageSubresourceRange& range);

private:    
    ErrType onCreate(VulkanDevice* pDevice, GraphicsResourceDescription& desc) override; 

    ErrType onGetMemoryRequirements(VulkanDevice* pDevice, VkMemoryRequirements& memRequirements) override;

    ErrType onBind(VulkanDevice* pDevice) override;
    
    ErrType onDestroy(VulkanDevice* pDevice) override;   

    VkFormatFeatureFlags loadFormatFeatures(VkImageCreateInfo& info, ResourceUsageFlags usage) const;

    VkImage         m_image;
    VkImageLayout   m_currentLayout;
    VkAccessFlags   m_currentAccessMask;
};
} // Recluse