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
    GraphicsResourceView**  ppRenderTargetViews;
    U32                     numRenderTargets;
    U32                     width;
    U32                     height;
    U32                     layers;
    GraphicsResourceView*   pDepthStencil;
};

class VulkanRenderPass
{
public:
    VulkanRenderPass()
        : m_fbo(VK_NULL_HANDLE)
        , m_renderPass(VK_NULL_HANDLE)
        , m_renderArea({ })
        , m_numRenderTargets(0)
        , m_fboId(0ull) { }

    ResultCode         initialize(VulkanDevice* pDevice, const VulkanRenderPassDesc& desc);
    ResultCode         release(VulkanDevice* pDevice);

    VkRenderPass    get() const { return m_renderPass; }
    VkFramebuffer   getFbo() const { return m_fbo; }

    U32             getNumRenderTargets() const { return m_numRenderTargets; }
    VkRect2D        getRenderArea() const { return m_renderArea; }

private:
    VkRenderPass    m_renderPass;
    VkFramebuffer   m_fbo;
    VkRect2D        m_renderArea;
    Hash64          m_fboId;
    U32             m_numRenderTargets;
};


namespace RenderPasses {

typedef Hash64 RenderPassId;
typedef Hash64 FrameBufferId;

VulkanRenderPass*       makeRenderPass(VulkanDevice* pDevice, U32 numRenderTargets, GraphicsResourceView** ppRenderTargetViews, GraphicsResourceView* pDepthStencil);
void                    clearCache(VulkanDevice* pDevice);
} // RenderPassManager


namespace DescriptorSets {


struct ConstantBuffer
{
    VulkanBuffer* buffer;
    VkDeviceSize offset;
    VkDeviceSize sizeBytes;
};

struct Structure
{
    VulkanResourceView**    ppShaderResources;
    VulkanResourceView**    ppUnorderedAccesses;
    ConstantBuffer*         ppConstantBuffers;
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

VkDescriptorSetLayout       makeLayout(VulkanContext* pContext, const Structure& structure);
const VulkanDescriptorAllocation& makeDescriptorSet(VulkanContext* pContext, const Structure& structure);
ResultCode                     releaseLayout(VulkanContext* pContext, const Structure& structure);
ResultCode                     releaseDescriptorSet(VulkanContext* pContext, const Structure& structure);
DescriptorSetLayoutId       obtainDescriptorLayoutKey(const Structure& structure);
void                        clearDescriptorSetCache(VulkanContext* pContext, ClearCacheFlag flag = ClearCacheFlag_DescriptorPoolFastClear);
void                        clearDescriptorLayoutCache(VulkanDevice* pContext);
} // DescriptorSet
} // Recluse