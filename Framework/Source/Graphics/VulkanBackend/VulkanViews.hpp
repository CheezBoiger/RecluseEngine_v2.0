//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class VulkanDevice;
class VulkanResource;

class VulkanResourceView : public GraphicsResourceView 
{
public:
    GraphicsAPI getApi() const override { return GraphicsApi_Vulkan; }

    VulkanResourceView(const ResourceViewDescription& desc)
        : GraphicsResourceView(desc)
        , m_expectedLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        , m_view(VK_NULL_HANDLE)
        , m_subresourceRange({ })
        , m_id(~0) { }

    ResultCode initialize(VulkanDevice* pDevice, VulkanResource* pResource);

    ResultCode release(VulkanDevice* pDevice);

    VkImageView get() const { return m_view; }

    VkImageLayout getExpectedLayout() const { return m_expectedLayout; }

    VkImageSubresourceRange getSubresourceRange() const { return m_subresourceRange; }
    ResourceViewId getId() const override { return m_id; }
    VulkanResource* getResource() const { return m_resource; }
private:
    static ResourceViewId kResourceViewCreationCounter;
    static MutexGuard kResourceViewCreationMutex;

public:
    void generateId() override 
    {
        ScopedLock _(kResourceViewCreationMutex);
        m_id = ++kResourceViewCreationCounter;
    }

private:

    VulkanResource*         m_resource;
    VkImageView             m_view;
    VkImageLayout           m_expectedLayout;
    VkImageSubresourceRange m_subresourceRange;
    ResourceViewId          m_id;
};


class VulkanSampler : public GraphicsSampler 
{
public:

    GraphicsAPI getApi() const override { return GraphicsApi_Vulkan; }

    VulkanSampler()
        : m_sampler(VK_NULL_HANDLE)
        , m_id(~0)
    {
    }

    virtual ~VulkanSampler() { }

    ResultCode initialize(VulkanDevice* pDevice, const SamplerDescription& desc);
    ResultCode release(VulkanDevice* pDevice);

    virtual SamplerDescription getDesc() override {
        return SamplerDescription();
    }

    VkSampler get() { return m_sampler; }

    VkSamplerCreateInfo getCopyInfo() const { return m_info; }
    SamplerId getId() const override { return m_id; }
private:
    static SamplerId kSamplerCreationCounter;
    static MutexGuard kSamplerCreationMutex;

public:
    void generateId() override
    {
        ScopedLock _(kSamplerCreationMutex);
        m_id = kSamplerCreationCounter++;
    }

private:
    VkSampler           m_sampler;
    VkSamplerCreateInfo m_info;
    SamplerId           m_id;
};

namespace ResourceViews {
ResourceViewId      makeResourceView(VulkanDevice* pDevice, VulkanResource* pResource, const ResourceViewDescription& desc);
VulkanSampler*      makeSampler(VulkanDevice* pDevice, const SamplerDescription& desc);
ResultCode          releaseResourceView(VulkanDevice* pDevice, ResourceViewId id);
ResultCode          releaseSampler(VulkanDevice* pDevice, SamplerId id);
VulkanResourceView* obtainResourceView(ResourceViewId id);
VulkanSampler*      obtainSampler(SamplerId);
} // ResourceViews
} // Recluse