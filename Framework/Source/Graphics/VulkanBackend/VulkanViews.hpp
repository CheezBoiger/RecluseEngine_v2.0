//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {
namespace Vulkan {
class VulkanDevice;
class VulkanResource;

class VulkanResourceView : public GraphicsResourceView 
{
public:
    typedef U64 DescriptionId;
    struct DescriptionPacked
    {
        union
        {
            struct 
            {
                U32 dim         : 4,
                    viewType    : 2,
                    pad0        : 2,
                    baseLayer   : 4,
                    baseMip     : 4,
                    layerCount  : 4,
                    mipCount    : 4,
                    pad1        : 8;
                U32 format;
            };
            DescriptionId hash0;
        };
    };

    GraphicsAPI                 getApi() const override { return GraphicsApi_Vulkan; }

    VulkanResourceView(const ResourceViewDescription& desc, Bool isBufferView)
        : GraphicsResourceView(desc)
        , m_descId(0)
        , m_id(~0)
        , m_isBufferView(isBufferView) { }

    virtual ~VulkanResourceView() { }

    ResultCode          initialize(VulkanDevice* pDevice, VulkanResource* pResource) 
    {
        generateDescriptionId();
        m_resource = pResource;
        return onInitialize(pDevice, pResource); 
    }

    ResultCode          release(VulkanDevice* pDevice) 
    { 
        return onRelease(pDevice); 
    }

    ResourceViewId              getId() const override { return m_id; }
    DescriptionId               getDescriptionId() const { return m_descId; }
    VulkanResource*             getResource() const { return m_resource; }
    Bool                        isBufferView() const { return m_isBufferView; }

protected:
    virtual ResultCode          onInitialize(VulkanDevice* pDevice, VulkanResource* pResource) { return RecluseResult_Ok; }
    virtual ResultCode          onRelease(VulkanDevice* pDevice) { return RecluseResult_Ok; }

private:
    static ResourceViewId       kResourceViewCreationCounter;
    static MutexGuard           kResourceViewCreationMutex;

public:
    void generateId() override 
    {
        ScopedLock _(kResourceViewCreationMutex);
        m_id = ++kResourceViewCreationCounter;
    }

private:
    void generateDescriptionId();

    VulkanResource*         m_resource;
    ResourceViewId          m_id;
    DescriptionId           m_descId;
    Bool                    m_isBufferView;
};


class VulkanImageView : public VulkanResourceView
{
public:
    VulkanImageView(const ResourceViewDescription& desc)
        : VulkanResourceView(desc, false)
        , m_expectedLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        , m_view(VK_NULL_HANDLE)
        , m_subresourceRange({ }) { }

    ResultCode onInitialize(VulkanDevice* pDevice, VulkanResource* pResource) override;
    ResultCode onRelease(VulkanDevice* pDevice) override;

    VkImageView                 get() const { return m_view; }
    VkImageLayout               getExpectedLayout() const { return m_expectedLayout; }
    VkImageSubresourceRange     getSubresourceRange() const { return m_subresourceRange; }

private:
    VkImageView                 m_view;
    VkImageLayout               m_expectedLayout;
    VkImageSubresourceRange     m_subresourceRange;
};


class VulkanSampler : public GraphicsSampler 
{
public:
    typedef Hash64 SamplerDescriptionId;

    struct SamplerDescriptionUnion
    {
        union
        {
            struct
            {
                U32 addrU       : 3,
                    addrV       : 3,
                    addrW       : 3,
                    borderColor : 2,
                    mipMapMode  : 1,
                    minFilter   : 2,
                    maxFilter   : 2,
                    compareOp   : 3,
                    pad0        : 13;
            };
            U32 hash0;
        };
        F32 minLod;
        F32 maxLod;
        F32 maxAnisotropy;
        F32 mipLodBias;
    };
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
    SamplerDescriptionId getDescriptionId() const { return m_descId; }
private:
    static SamplerId kSamplerCreationCounter;
    static MutexGuard kSamplerCreationMutex;

public:
    void generateId() override
    {
        ScopedLock _(kSamplerCreationMutex);
        m_id = kSamplerCreationCounter++;
    }

    void generateDescriptionId(const SamplerDescription& desc);

private:
    VkSampler               m_sampler;
    VkSamplerCreateInfo     m_info;
    SamplerId               m_id;
    Hash64                  m_descId;
};

namespace ResourceViews {
ResourceViewId      makeResourceView(VulkanDevice* pDevice, VulkanResource* pResource, const ResourceViewDescription& desc);
VulkanSampler*      makeSampler(VulkanDevice* pDevice, const SamplerDescription& desc);
ResultCode          releaseResourceView(VulkanDevice* pDevice, ResourceViewId id);
ResultCode          releaseSampler(VulkanDevice* pDevice, SamplerId id);
VulkanResourceView* obtainResourceView(DeviceId deviceId, ResourceViewId id);
VulkanSampler*      obtainSampler(DeviceId deviceId, SamplerId sampler);
void                clearCache(VulkanDevice* pDevice);
} // ResourceViews
} // Vulkan
} // Recluse