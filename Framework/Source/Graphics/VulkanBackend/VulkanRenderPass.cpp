//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include <vector>

namespace Recluse {

std::unordered_map<Hash64, RefCount<VkFramebuffer>> g_fboCache;
std::unordered_map<Hash64, VkRenderPass> g_rpCache;

static U64 serialize(const VkFramebufferCreateInfo& info)
{
    Hash64 uniqueId = 0ull;

    uniqueId += static_cast<U64>( info.attachmentCount + info.width + info.height );
    uniqueId += static_cast<U64>( info.layers + info.flags );

    for (U32 i = 0; i < info.attachmentCount; ++i)
    {
        VkImageView ref = info.pAttachments[i];
        uniqueId += reinterpret_cast<Hash64>(ref);
    }

    // twice hash.
    uniqueId = recluseHash(&uniqueId, sizeof(Hash64));
    uniqueId = recluseHash(&uniqueId, sizeof(Hash64));
    return uniqueId;
}


static U64 serialize(const VkRenderPassCreateInfo& info)
{
    return 0ull;
}


static Bool inFboCache(Hash64 fboId)
{
    auto it = g_fboCache.find(fboId);
    if (it == g_fboCache.end())
        return false;
    return it->second.hasRefs();
}


static Bool cacheFbo(Hash64 fboId, const VkFramebuffer fbo)
{
    if (inFboCache(fboId))
    {
        R_VERBOSE(R_CHANNEL_VULKAN, "Fbo is already in cache! Can not cache unless deleting the existing one...");
        return false;
    }

    g_fboCache[fboId] = fbo;
    g_fboCache[fboId].addRef();

    return true;
}


static void addFboRef(Hash64 fboId)
{
    g_fboCache[fboId].addRef();
}

static VkFramebuffer getCachedFbo(Hash64 fboId)
{
    return g_fboCache[fboId]();
}


static void destroyFbo(Hash64 fboId, VkDevice device)
{
    if (!g_fboCache[fboId].release())
    {
        VkFramebuffer fbo = g_fboCache[fboId].getData();
        vkDestroyFramebuffer(device, fbo, nullptr);
    }
}


ErrType VulkanRenderPass::initialize(VulkanDevice* pDevice, const RenderPassDesc& desc)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Creating vulkan render pass...");

    VkAttachmentDescription descriptions[9]; // including depthstencil, if possible.
    VkAttachmentReference   references[9];  // references for subpass.
    VkImageView viewAttachments[9];
    VkSubpassDependency dependencies[2];    // dependecies between renderpasses.
    B32 depthStencilIncluded            = desc.pDepthStencil ? true : false;
    VkResult result                     = VK_SUCCESS;
    VkFramebufferCreateInfo fboIf       = { };
    VkRenderPassCreateInfo  rpIf        = { };
    VkSubpassDescription subpass        = { };
    U32 totalNumAttachments             = 0;

    auto storeColorDescription = [&] (U32 i) -> void 
    {
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
        
        references[i].attachment        = i;
        references[i].layout            = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        viewAttachments[i]              = pView->get();
    };

    auto storeDepthStencilDescription = [&] (U32 i) -> void 
    {
        VulkanResourceView* pView   = static_cast<VulkanResourceView*>(desc.pDepthStencil);
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
    
        references[i].attachment        = i;
        references[i].layout            = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        
        viewAttachments[i]              = pView->get();
    };
    
    U32 i = 0;
    for (i = 0; i < desc.numRenderTargets; ++i) 
    {
        storeColorDescription(i);
    }

    if (depthStencilIncluded) 
    {
        storeDepthStencilDescription(i);
    }

    totalNumAttachments             = desc.numRenderTargets + depthStencilIncluded;

    subpass.colorAttachmentCount    = desc.numRenderTargets;
    subpass.pColorAttachments       = references;
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = depthStencilIncluded ? &references[i] : nullptr;

    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0; // Our first pass is our renderpass.
    dependencies[0].srcAccessMask   = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL; // Our first pass is our renderpass.
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstAccessMask   = 0;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (depthStencilIncluded) 
    {
        dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependencies[1].srcAccessMask |= dependencies[1].dstAccessMask;
    }


    rpIf.sType                      = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpIf.attachmentCount            = totalNumAttachments;
    rpIf.pAttachments               = descriptions;
    rpIf.subpassCount               = 1;
    rpIf.pSubpasses                 = &subpass;
    rpIf.dependencyCount            = 2;
    rpIf.pDependencies              = dependencies;
    
    
    result = vkCreateRenderPass(pDevice->get(), &rpIf, nullptr, &m_renderPass);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan render pass!");

        destroy(pDevice);

        return RecluseResult_Failed;
    }

    fboIf.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fboIf.renderPass        = m_renderPass;
    fboIf.width             = desc.width;
    fboIf.height            = desc.height;
    fboIf.pAttachments      = viewAttachments;
    fboIf.attachmentCount   = totalNumAttachments;
    fboIf.layers            = 1; // For future use.

    // Find a proper framebuffer object in the cache, otherwise if miss, create a new fbo.
    Hash64 fboId = serialize(fboIf);
    if (inFboCache(fboId))
    {
        R_VERBOSE(R_CHANNEL_VULKAN, "Fbo Cache hit!");
        m_fbo = getCachedFbo(fboId);

        addFboRef(fboId);
    }
    else
    {
        R_VERBOSE(R_CHANNEL_VULKAN, "No cached fbo was hit, creating a new fbo...");
        result = vkCreateFramebuffer(pDevice->get(), &fboIf, nullptr, &m_fbo);
        
        if (result != VK_SUCCESS) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan framebuffer object.!");

            destroy(pDevice);

            return RecluseResult_Failed;
        }

        cacheFbo(fboId, m_fbo);
    }

    m_desc              = desc;
    m_renderArea        = { };
    m_renderArea.extent = { desc.width, desc.height };
    m_renderArea.offset = { 0, 0 };
    m_fboId             = fboId;

    return RecluseResult_Ok;
}


ErrType VulkanRenderPass::destroy(VulkanDevice* pDevice)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying render pass...");

    if (m_renderPass) 
    {
        vkDestroyRenderPass(pDevice->get(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;    

    }

    if (m_fbo) 
    {
        destroyFbo(m_fboId, pDevice->get());
        m_fbo = VK_NULL_HANDLE;
    }

    return RecluseResult_Ok;
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