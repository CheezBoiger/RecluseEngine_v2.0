//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"

#include "Recluse/Messaging.hpp"
#include <vector>

namespace Recluse {


ErrType VulkanRenderPass::initialize(VulkanDevice* pDevice, const RenderPassDesc& desc)
{
    VkAttachmentDescription descriptions[9]; // including depthstencil, if possible.
    VkImageView viewAttachments[9];
    B32 depthStencilIncluded            = desc.pDepthStencil ? true : false;
    VkResult result                     = VK_SUCCESS;
    VkFramebufferCreateInfo fboIf       = { };
    VkRenderPassCreateInfo  rpIf        = { };
    U32 totalNumAttachments             = 0;

    auto storeDescription = [&] (U32 i) -> void {
        VulkanResourceView* pView   = static_cast<VulkanResourceView*>(desc.ppRenderTargetViews[i]);
        ResourceViewDesc viewDesc   = pView->getDesc();

        descriptions[i].samples         = VK_SAMPLE_COUNT_1_BIT;
        descriptions[i].format          = Vulkan::getVulkanFormat(viewDesc.format);
        descriptions[i].initialLayout   = pView->getExpectedLayout();
        descriptions[i].finalLayout     = pView->getExpectedLayout();
        descriptions[i].loadOp          = VK_ATTACHMENT_LOAD_OP_LOAD;
        descriptions[i].storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        descriptions[i].stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        descriptions[i].stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        descriptions[i].flags           = 0;

        viewAttachments[i] = pView->get();
    };
    
    U32 i = 0;
    for (i = 0; i < desc.numRenderTargets; ++i) {

        storeDescription(i);

    }

    if (depthStencilIncluded) {

        storeDescription(i);

    }

    totalNumAttachments             = desc.numRenderTargets + depthStencilIncluded;

    rpIf.sType                      = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpIf.attachmentCount            = totalNumAttachments;
    rpIf.pAttachments               = descriptions;
    
    result = vkCreateRenderPass(pDevice->get(), &rpIf, nullptr, &m_renderPass);

    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan render pass!");

        destroy(pDevice);

        return REC_RESULT_FAILED;
    
    }

    fboIf.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fboIf.renderPass        = m_renderPass;
    fboIf.width             = desc.width;
    fboIf.height            = desc.height;
    fboIf.pAttachments      = viewAttachments;
    fboIf.attachmentCount   = totalNumAttachments;
    fboIf.layers            = 1; // For future use.

    result = vkCreateFramebuffer(pDevice->get(), &fboIf, nullptr, &m_fbo);

    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan framebuffer object.!");

        destroy(pDevice);

        return REC_RESULT_FAILED;
    
    }

    m_desc = desc;

    return REC_RESULT_OK;
}


ErrType VulkanRenderPass::destroy(VulkanDevice* pDevice)
{
    if (m_renderPass) {

        vkDestroyRenderPass(pDevice->get(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;    

    }

    if (m_fbo) {
    
        vkDestroyFramebuffer(pDevice->get(), m_fbo, nullptr);
        m_fbo = VK_NULL_HANDLE;
    
    }

    return REC_RESULT_OK;
}


GraphicsResourceView* VulkanRenderPass::getRenderTarget(U32 idx)
{
    return m_desc.ppRenderTargetViews[idx];
}


GraphicsResourceView* VulkanRenderPass::getDepthStencil()
{
    return m_desc.pDepthStencil;
}
} // Recluse