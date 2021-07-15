//
#pragma once

#include "VulkanCommons.hpp"

#include "Recluse/Graphics/RenderPass.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"


namespace Recluse {

class VulkanDevice;

class VulkanRenderPass : public RenderPass {
public:
    VulkanRenderPass()
        : m_fbo(VK_NULL_HANDLE)
        , m_renderPass(VK_NULL_HANDLE) { }

    ErrType initialize(VulkanDevice* pDevice, const RenderPassDesc& desc);

    ErrType destroy(VulkanDevice* pDevice);

    VkRenderPass get() const { return m_renderPass; }
    VkFramebuffer getFbo() const { return m_fbo; }

    GraphicsResourceView* getRenderTarget(U32 idx) override;
    GraphicsResourceView* getDepthStencil() override;

private:
    VkRenderPass    m_renderPass;
    VkFramebuffer   m_fbo;

    RenderPassDesc m_desc;
};


class VulkanDescriptorSetLayout : public DescriptorSetLayout {
public:
    VulkanDescriptorSetLayout()
        : m_layout(VK_NULL_HANDLE) { }

    ErrType initialize(VulkanDevice* pDevice, const DescriptorSetLayoutDesc& desc);
    ErrType destroy(VulkanDevice* pDevice);

    VkDescriptorSetLayout get() const { return m_layout; }

private:
    VkDescriptorSetLayout m_layout;
};
} // Recluse