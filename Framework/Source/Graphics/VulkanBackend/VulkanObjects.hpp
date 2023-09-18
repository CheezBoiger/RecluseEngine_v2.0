//
#pragma once

#include "VulkanCommons.hpp"
#include "VulkanDescriptorManager.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"


namespace Recluse {

class VulkanDevice;
class VulkanResourceView;
class VulkanSampler;
class VulkanResource;
class VulkanBuffer;
class VulkanContext;

struct VulkanRenderPassDesc
{
    ResourceViewId*  ppRenderTargetViews;
    U32              numRenderTargets;
    U32              width;
    U32              height;
    U32              layers;
    ResourceViewId   pDepthStencil;
};

struct FramebufferObject
{
    VkFramebuffer framebuffer;
    VkRect2D      renderArea;
#if defined(USE_STD_LRU_IMPL)
    Hash64        id;
    U32           age;
#endif
};

class VulkanRenderPass
{
public:
    VulkanRenderPass(FramebufferObject* framebuffer = nullptr, VkRenderPass renderPass = VK_NULL_HANDLE)
        : m_fbo(framebuffer)
        , m_renderPass(renderPass) { }

    VkRenderPass    get() const { return m_renderPass; }
    VkFramebuffer   getFbo() const { return m_fbo->framebuffer; }
    const VkRect2D& getRenderArea() const { return m_fbo->renderArea; }
    Bool            isNull() const { return (!m_renderPass && !m_fbo); }

    // Nullify the render pass, it's okay to do this since this is just a temporary container.
    void            nullify() { m_fbo = nullptr; m_renderPass = VK_NULL_HANDLE; }
private:
    VkRenderPass        m_renderPass;
    FramebufferObject*  m_fbo;
};


namespace RenderPasses {

typedef Hash64 RenderPassId;
typedef Hash64 FrameBufferId;

VulkanRenderPass        makeRenderPass(VulkanDevice* pDevice, U32 numRenderTargets, ResourceViewId* ppRenderTargetViews, ResourceViewId pDepthStencil);
void                    clearCache(VulkanDevice* pDevice);
void                    updateTick();
void                    checkLruCache(VkDevice device);
} // RenderPass


namespace DescriptorSets {


struct BufferView
{
    VulkanBuffer* buffer;
    U32 offset;
    U32 sizeBytes;
};


struct Structure
{
    VulkanResourceView**    ppShaderResources;
    VulkanResourceView**    ppUnorderedAccesses;
    BufferView*             ppConstantBuffers;
    VulkanSampler**         ppSamplers;
    union
    {
        struct
        {
            U16             uavs;  // 16
            U16             srvs;  // 32
            ShaderStageFlags shaderTypeFlags; // 64
            U16             samplers;           
            U16             constantBuffers;
            U32             pad0;
        } value;
        struct
        {
            U64 l0;
            U64 l1;
        } hash;
    } key;

    Structure()
        : ppSamplers(nullptr)
        , ppUnorderedAccesses(nullptr)
        , ppConstantBuffers(nullptr)
        , ppShaderResources(nullptr)
    {
        key.hash.l0 = 0;
        key.hash.l1 = 0;
    }
};

enum ClearCacheFlag
{
    ClearCacheFlag_DescriptorPoolFastClear,
    ClearCacheFlag_IndividualDescriptorSetClear
};

typedef Hash64 DescriptorSetLayoutId;
typedef Hash64 DescriptorSetId;

VkDescriptorSetLayout               makeLayout(VulkanContext* pContext, const Structure& structure);
const VulkanDescriptorAllocation&   makeDescriptorSet(VulkanContext* pContext, const Structure& structure);
ResultCode                          releaseLayout(VulkanContext* pContext, const Structure& structure);
ResultCode                          releaseDescriptorSet(VulkanContext* pContext, const Structure& structure);
DescriptorSetLayoutId               obtainDescriptorLayoutKey(const Structure& structure);
void                                clearDescriptorSetCache(VulkanContext* pContext, ClearCacheFlag flag = ClearCacheFlag_DescriptorPoolFastClear);
void                                clearDescriptorLayoutCache(VulkanDevice* pContext);
} // DescriptorSet
} // Recluse