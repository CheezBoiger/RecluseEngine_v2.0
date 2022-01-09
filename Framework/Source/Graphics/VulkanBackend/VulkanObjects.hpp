//
#pragma once

#include "VulkanCommons.hpp"

#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"


namespace Recluse {

class VulkanDevice;

class VulkanRenderPass : public RenderPass 
{
public:
    VulkanRenderPass()
        : m_fbo(VK_NULL_HANDLE)
        , m_renderPass(VK_NULL_HANDLE)
        , m_renderArea({ })
        , m_desc({ }) { }

    ErrType initialize(VulkanDevice* pDevice, const RenderPassDesc& desc);

    ErrType destroy(VulkanDevice* pDevice);

    VkRenderPass get() const { return m_renderPass; }
    VkFramebuffer getFbo() const { return m_fbo; }

    GraphicsResourceView* getRenderTarget(U32 idx) override;
    GraphicsResourceView* getDepthStencil() override;
    U32 getNumRenderTargets() const override { return m_desc.numRenderTargets; }

    VkRect2D getRenderArea() const { return m_renderArea; }

private:
    VkRenderPass    m_renderPass;
    VkFramebuffer   m_fbo;
    VkRect2D        m_renderArea;

    RenderPassDesc  m_desc;
};


class VulkanDescriptorSetLayout : public DescriptorSetLayout 
{
public:
    VulkanDescriptorSetLayout()
        : m_layout(VK_NULL_HANDLE) { }

    ErrType initialize(VulkanDevice* pDevice, const DescriptorSetLayoutDesc& desc);
    ErrType destroy(VulkanDevice* pDevice);

    VkDescriptorSetLayout get() const { return m_layout; }

private:
    VkDescriptorSetLayout m_layout;
};


class VulkanDescriptorSet : public DescriptorSet 
{
public:
    VulkanDescriptorSet()
        : m_set(VK_NULL_HANDLE)
        , m_pDevice(nullptr) { }

    ErrType initialize(VulkanDevice* pDevice, VulkanDescriptorSetLayout* pLayout);

    ErrType destroy();

    ErrType update(DescriptorSetBind* pBinds, U32 bindCount) override;

    VkDescriptorSet get() const { return m_set; }

private:
    VkDescriptorSet m_set;
    VulkanDevice*   m_pDevice;
};
} // Recluse