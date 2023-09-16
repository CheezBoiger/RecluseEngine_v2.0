//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include <vector>
#include <list>

namespace Recluse {


struct RenderPassLiveObject
{
    ReferenceCounter<VkRenderPass>  rp;
    RenderPasses::RenderPassId      id;
    U32                             age;
};

std::list<RenderPassLiveObject*>                                                             g_rpList;
std::list<FramebufferObject*>                                                                g_fbList;
std::unordered_map<RenderPasses::RenderPassId, std::list<RenderPassLiveObject*>::iterator>   g_rpCache;
std::unordered_map<Hash64, std::list<FramebufferObject*>::iterator>                          g_fbCache;

// Tick occurs every new iteration of the engine. This needs to increment every new frame.
U32                                                                                          g_tick = 0;

R_DECLARE_GLOBAL_U32(g_renderPassMaxAge, 12, "Vulkan.RenderPassMaxAge");
R_DECLARE_GLOBAL_U32(g_frameBufferMaxAge, 12, "Vulkan.FramebufferMaxAge");

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
void cacheRenderPass(RenderPasses::RenderPassId renderpassId, VkRenderPass renderPass)
{
    RenderPassLiveObject* obj = new RenderPassLiveObject();
    obj->rp = renderPass;
    obj->id = renderpassId;
    obj->age = g_tick;
    g_rpList.push_front(obj);
    g_rpCache.insert(std::make_pair(renderpassId, g_rpList.begin()));
}


R_INTERNAL 
VkRenderPass referRenderPass(RenderPasses::RenderPassId renderPassId)
{
    if (g_rpCache.find(renderPassId) != g_rpCache.end())
    {
        RenderPassLiveObject* obj = nullptr;
        obj = *g_rpCache[renderPassId];
        g_rpList.erase(g_rpCache[renderPassId]);
        obj->age = g_tick;

        g_rpList.push_front(obj);
        g_rpCache[renderPassId] = g_rpList.begin();
        return obj->rp();
    }
    return nullptr; 
}


R_INTERNAL 
U64 serialize(const VkRenderPassCreateInfo& info)
{
    return 0ull;
}


R_INTERNAL
Bool inFboCache(Hash64 fboId)
{
    auto it = g_fbCache.find(fboId);
    if (it == g_fbCache.end())
        return false;
    return true;
}


R_INTERNAL
FramebufferObject* getCachedFbo(Hash64 fboId)
{
    if (g_fbCache.find(fboId) != g_fbCache.end())
    {
        FramebufferObject* obj = *g_fbCache[fboId];
        g_fbList.erase(g_fbCache[fboId]);
        obj->age = g_tick;
        g_fbList.push_front(obj);
        g_fbCache[fboId] = g_fbList.begin();
        return obj;
    }
    return nullptr;
}


R_INTERNAL
FramebufferObject* cacheFbo(Hash64 fboId, VkFramebuffer fbo, const VkRect2D& renderArea, U32 numReferences)
{
    FramebufferObject* data = new FramebufferObject();
    data->framebuffer = fbo;
    data->renderArea = renderArea;
    data->age = g_tick;
    data->id = fboId;
    g_fbList.push_front(data);
    g_fbCache.insert(std::make_pair(fboId, g_fbList.begin()));
    return data;
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
        VulkanImageView* pView = ResourceViews::obtainResourceView(desc.pDepthStencil)->castTo<VulkanImageView>();
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
        references[i].layout            = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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
        dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].srcAccessMask |= dependencies[0].dstAccessMask;

        dependencies[0].srcStageMask    |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].dstStageMask    |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[1].srcStageMask    |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask    |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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
    auto iter = g_rpCache.find(renderPassId);
    // Failed to find a suitable render pass, make a new one.
    if (iter == g_rpCache.end())
    {
        VkRenderPass renderPass = createRenderPass(pDevice, desc);
        U32 references = desc.numRenderTargets + ((desc.pDepthStencil != 0) ? 1 : 0);
        cacheRenderPass(renderPassId, renderPass);
        (*g_rpCache[renderPassId])->rp.addReference(references);
        return (*g_rpCache[renderPassId])->rp();
    }
    else
    {
        // We have one, let's return this one.
        return referRenderPass(renderPassId);
    }
}


namespace RenderPasses {


void checkLruCache(VkDevice device)
{
    const U32 currentTick = g_tick;

    // Check Render Pass cache.
    if (!g_rpList.empty())
    {
        RenderPassLiveObject* obj = g_rpList.back();
        const U32 ageRate = g_tick - obj->age;
        if (ageRate >= g_renderPassMaxAge)
        {
            R_DEBUG("Vulkan", "Destroy Render Pass.");
            g_rpList.pop_back();
            g_rpCache.erase(obj->id);
            VkRenderPass rp = obj->rp();
            vkDestroyRenderPass(device, rp, nullptr);
            delete obj;
        }
    }

    // Now check fbo cache.
    if (!g_fbList.empty())
    {
        FramebufferObject* obj = g_fbList.back();
        const U32 ageRate = g_tick - obj->age;
        if (ageRate >= g_renderPassMaxAge)
        {
            R_DEBUG("Vulkan", "Cleaning up framebuffer.");
            g_fbList.pop_back();
            g_fbCache.erase(obj->id);
            VkFramebuffer fbo = obj->framebuffer;
            vkDestroyFramebuffer(device, fbo, nullptr);
            delete obj;
        }
    }
}


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
    for (auto iter : g_rpList)
    {
        while (iter->rp.release());
        vkDestroyRenderPass(pDevice->get(), iter->rp(), nullptr);
        delete iter;
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying render pass...");
    }

    for (auto iter : g_fbList)
    {
        vkDestroyFramebuffer(pDevice->get(), iter->framebuffer, nullptr);
        delete iter;
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying framebuffer");
    }
    g_fbList.clear();
    g_fbCache.clear();
    g_rpList.clear();
    g_rpCache.clear();
}


void updateTick()
{
    g_tick += 1;
}
} // RenderPass
} // Recluse