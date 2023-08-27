//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include <vector>

namespace Recluse {


std::unordered_map<Hash64, ReferenceCounter<FramebufferObject>> g_fboCache;
std::unordered_map<Hash64, ReferenceCounter<VkRenderPass>> g_rpCache;

R_INTERNAL 
U64 serialize(const VkFramebufferCreateInfo& info)
{
    Hash64 uniqueId = 0ull;

    uniqueId += static_cast<U64>( info.attachmentCount + info.width + info.height );
    uniqueId += static_cast<U64>( info.layers + info.flags );

    for (U32 i = 0; i < info.attachmentCount; ++i)
    {
        VkImageView ref = info.pAttachments[i];
        uniqueId ^= reinterpret_cast<Hash64>(ref);
    }

    uniqueId = recluseHashFast(&uniqueId, sizeof(Hash64));
    return uniqueId;
}


R_INTERNAL 
U64 serialize(const VkRenderPassCreateInfo& info)
{
    return 0ull;
}


R_INTERNAL
Bool inFboCache(Hash64 fboId)
{
    auto it = g_fboCache.find(fboId);
    if (it == g_fboCache.end())
        return false;
    return it->second.hasReferences();
}


R_INTERNAL
FramebufferObject* getCachedFbo(Hash64 fboId)
{
    return &g_fboCache[fboId]();
}


R_INTERNAL
FramebufferObject* cacheFbo(Hash64 fboId, VkFramebuffer fbo, const VkRect2D& renderArea, U32 numReferences)
{
    if (inFboCache(fboId))
    {
        R_VERBOSE(R_CHANNEL_VULKAN, "Fbo is already in cache! Can not cache unless deleting the existing one...");
        return getCachedFbo(fboId);
    }

    FramebufferObject data = { fbo, renderArea };
    g_fboCache[fboId] = data;
    g_fboCache[fboId].addReference(numReferences);

    return &g_fboCache[fboId]();
}


R_INTERNAL 
void destroyFbo(Hash64 fboId, VkDevice device)
{
    if (!g_fboCache[fboId].release())
    {
        VkFramebuffer fbo = g_fboCache[fboId]().framebuffer;
        vkDestroyFramebuffer(device, fbo, nullptr);
        g_fboCache.erase(fboId);
    }
}


FramebufferObject* makeFrameBuffer(VulkanDevice* pDevice, VkRenderPass renderPass, const VulkanRenderPassDesc& desc)
{
    
    FramebufferObject* framebuffer = nullptr;
    VkFramebufferCreateInfo fboIf = { };
    VkImageView viewAttachments[9];
    Bool hasDepthStencil = false;
    U32 totalAttachmentCount = 0;    
    U32 i = 0;
    for (i = 0; i < desc.numRenderTargets; ++i)
    {
        VulkanImageView* pView = ResourceViews::obtainResourceView(desc.ppRenderTargetViews[i])->castTo<VulkanImageView>();
        viewAttachments[i] = pView->get();
    }

    if (desc.pDepthStencil)
    {
        VulkanImageView* pView = ResourceViews::obtainResourceView(desc.ppRenderTargetViews[i])->castTo<VulkanImageView>();
        viewAttachments[i] = pView->get();
        hasDepthStencil = true;
    }

    totalAttachmentCount = desc.numRenderTargets + hasDepthStencil;

    fboIf.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fboIf.renderPass        = renderPass;
    fboIf.width             = desc.width;
    fboIf.height            = desc.height;
    fboIf.pAttachments      = viewAttachments;
    fboIf.attachmentCount   = totalAttachmentCount;
    fboIf.layers            = desc.layers;

    // Find a proper framebuffer object in the cache, otherwise if miss, create a new fbo.
    Hash64 fboId = serialize(fboIf);
    if (inFboCache(fboId))
    {
        framebuffer = getCachedFbo(fboId);
    }
    else
    {
        ResultCode result = RecluseResult_Ok;
        R_VERBOSE(R_CHANNEL_VULKAN, "No cached fbo was hit, creating a new fbo...");
        VkFramebuffer fbo = VK_NULL_HANDLE;
        result = vkCreateFramebuffer(pDevice->get(), &fboIf, nullptr, &fbo);
        
        if (result != VK_SUCCESS) 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Failed to create vulkan framebuffer object.!");
            return VK_NULL_HANDLE;
        }
        else
        {   
            VkRect2D renderArea = { };
            renderArea.extent = { desc.width, desc.height };
            renderArea.offset = { 0, 0 };
            framebuffer = cacheFbo(fboId, fbo, renderArea, totalAttachmentCount);
        }
    }

    return framebuffer;
}


VkRenderPass createRenderPass(VulkanDevice* pDevice,  const VulkanRenderPassDesc& desc)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Creating vulkan render pass...");
    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkAttachmentDescription descriptions[9]; // including depthstencil, if possible.
    VkAttachmentReference   references[9];  // references for subpass.
    VkSubpassDependency dependencies[2];    // dependecies between renderpasses.
    B32 depthStencilIncluded            = desc.pDepthStencil ? true : false;
    VkResult result                     = VK_SUCCESS;
    VkRenderPassCreateInfo  rpIf        = { };
    VkSubpassDescription subpass        = { };
    U32 totalNumAttachments             = 0;

    auto storeColorDescription = [&] (U32 i) -> void 
    {
        VulkanImageView* pView           = ResourceViews::obtainResourceView(desc.ppRenderTargetViews[i])->castTo<VulkanImageView>();
        ResourceViewDescription viewDesc    = pView->getDesc();

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
    };

    auto storeDepthStencilDescription = [&] (U32 i) -> void 
    {
        VulkanImageView* pView   = ResourceViews::obtainResourceView(desc.pDepthStencil)->castTo<VulkanImageView>();
        ResourceViewDescription viewDesc   = pView->getDesc();

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
    
    result = vkCreateRenderPass(pDevice->get(), &rpIf, nullptr, &renderPass);

    return renderPass;
}


R_INTERNAL
VkRenderPass internalMakeRenderPass(VulkanDevice* pDevice,  const VulkanRenderPassDesc& desc, RenderPasses::RenderPassId renderPassId)
{
    auto& iter = g_rpCache.find(renderPassId);
    // Failed to find a suitable render pass, make a new one.
    if (iter == g_rpCache.end())
    {
        VkRenderPass renderPass = createRenderPass(pDevice, desc);
        g_rpCache.insert(std::make_pair(renderPassId, renderPass));
        U32 references = desc.numRenderTargets + ((desc.pDepthStencil != 0) ? 1 : 0);
        g_rpCache[renderPassId].addReference(references);
        return g_rpCache[renderPassId]();
    }
    else
    {
        // We have one, let's return this one.
        return iter->second();
    }
}


namespace RenderPasses {

VulkanRenderPass makeRenderPass(VulkanDevice* pDevice, U32 numRenderTargets, ResourceViewId* ppRenderTargetViews, ResourceViewId pDepthStencil)
{
    // We never really have a max of 8 render targets in current hardware.
    R_ASSERT(numRenderTargets <= 8);
    R_ASSERT(pDevice != NULL);
    thread_local VulkanResourceView::DescriptionId ids[9];    
    VulkanRenderPass* pPass = nullptr;
    Hash64 accumulate       = 0;
    U32 count           = 0;
    U32 targetWidth     = 0;
    U32 targetHeight    = 0;
    U32 targetLayers    = 0;
    
    for (U32 i = 0; i < numRenderTargets; ++i)
    {
        VulkanResourceView* pView = ResourceViews::obtainResourceView(ppRenderTargetViews[i]);
        R_ASSERT(!pView->getResource()->isBuffer());
        VulkanImage* pImage = pView->getResource()->castTo<VulkanImage>();
        ids[count++]    = pView->getDescriptionId();
        targetWidth     = Math::maximum(targetWidth, pImage->getWidth());
        targetHeight    = Math::maximum(targetHeight, pImage->getHeight());
        targetLayers    = Math::maximum(targetLayers, pImage->getDepthOrArraySize());
    }

    if (pDepthStencil)
    {
        VulkanResourceView* pView = ResourceViews::obtainResourceView(pDepthStencil);
        R_ASSERT(!pView->getResource()->isBuffer());
        VulkanImage* pImage = pView->getResource()->castTo<VulkanImage>();
        ids[count++]    = pView->getDescriptionId();
        targetWidth     = Math::maximum(targetWidth, pImage->getWidth());
        targetHeight    = Math::maximum(targetHeight, pImage->getHeight());
        targetLayers    = Math::maximum(targetLayers, pImage->getDepthOrArraySize());
    }

    // TODO(Garcia): We need to find a more general way to identify renderpasses,
    //               because this is relying too much on VkImageView (which is only meant for FBOs.)
    //               Due to this, we are creating too many pipeline caches for the same renderpass.

    const U64 resourceViewBytes = sizeof(VulkanResourceView::DescriptionId) * (U64)count;
    RenderPassId id = recluseHashFast(ids, resourceViewBytes);
    VulkanRenderPassDesc desc   = { };
    desc.numRenderTargets       = numRenderTargets;
    desc.pDepthStencil          = pDepthStencil;
    desc.ppRenderTargetViews    = ppRenderTargetViews;
    desc.width                  = targetWidth;
    desc.height                 = targetHeight;
    desc.layers                 = targetLayers;

    VkRenderPass renderPass         = internalMakeRenderPass(pDevice, desc, id);
    FramebufferObject* framebuffer  = makeFrameBuffer(pDevice, renderPass, desc);

    return VulkanRenderPass(framebuffer, renderPass);
}

void clearCache(VulkanDevice* pDevice)
{
    R_ASSERT(pDevice != NULL);
    for (auto iter : g_rpCache)
    {
        while (iter.second.release());
        vkDestroyRenderPass(pDevice->get(), iter.second(), nullptr);
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying render pass...");
    }

    for (auto iter : g_fboCache)
    {   
        while (iter.second.release());
        vkDestroyFramebuffer(pDevice->get(), iter.second().framebuffer, nullptr);
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying framebuffer");
    }
    g_rpCache.clear();
    g_fboCache.clear();
}
} // RenderPass
} // Recluse