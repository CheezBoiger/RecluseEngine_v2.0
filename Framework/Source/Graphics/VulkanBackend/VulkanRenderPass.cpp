//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "Graphics/LifetimeCache.hpp"

#include <vector>
#include <list>

namespace Recluse {


struct RenderPassLiveObject
{
    ReferenceCounter<VkRenderPass>  rp;
#if defined(USE_STD_LRU_IMPL)
    RenderPasses::RenderPassId      id;
    U32                             age;
#endif
};

// TODO: std::list calls malloc and free on nodes whenever we push objects to the fronst of the queue.
//       This can cause a performance drop, when we start to have many renderpasses and framebuffers.
//       We need to optimize this data structure to eliminate these mallocs and frees when possible.
//       An idea would be to create our own linked list, and perform assignments by moving data from those
//       nodes, and reassigning back to the top.
#if defined(USE_STD_LRU_IMPL)
std::list<std::unique_ptr<RenderPassLiveObject>>                                                            g_rpList;
std::unordered_map<RenderPasses::RenderPassId, std::list<std::unique_ptr<RenderPassLiveObject>>::iterator>  g_rpCache;
std::list<std::unique_ptr<FramebufferObject>>                                                               g_fbList;
std::unordered_map<Hash64, std::list<std::unique_ptr<FramebufferObject>>::iterator>                         g_fbCache;
// Tick occurs every new iteration of the engine. This needs to increment every new frame.
U32                                                                                                         g_tick = 0;
#else
LifetimeCache<Hash64, std::unique_ptr<FramebufferObject>>                                                   g_fbCache;
LifetimeCache<RenderPasses::RenderPassId, std::unique_ptr<RenderPassLiveObject>>                            g_rpCache;
#endif

R_DECLARE_GLOBAL_U32(g_renderPassMaxAge, 12, "Vulkan.RenderPassMaxAge");
R_DECLARE_GLOBAL_U32(g_frameBufferMaxAge, 12, "Vulkan.FramebufferMaxAge");

R_INTERNAL 
U64 serialize(const VkFramebufferCreateInfo& info)
{
    Hash64 uniqueId = 0ull;

    uniqueId += static_cast<Hash64>( info.attachmentCount ^ info.width ^ info.height );
    uniqueId += static_cast<Hash64>( info.layers ^ info.flags );

    uniqueId =  recluseHashFast(&uniqueId, sizeof(Hash64));
    uniqueId ^= recluseHashFast(info.pAttachments, sizeof(VkImageView) * info.attachmentCount);
    return uniqueId;
}


R_INTERNAL
void cacheRenderPass(RenderPasses::RenderPassId renderpassId, VkRenderPass renderPass)
{
    std::unique_ptr<RenderPassLiveObject> obj = std::make_unique<RenderPassLiveObject>();
    obj->rp = renderPass;
#if defined(USE_STD_LRU_IMPL)
    obj->id = renderpassId;
    obj->age = g_tick;
    g_rpList.push_front(std::move(obj));
    g_rpCache.insert(std::make_pair(renderpassId, g_rpList.begin()));
#else
    g_rpCache.insert(renderpassId, std::move(obj));
#endif
}


R_INTERNAL 
VkRenderPass referRenderPass(RenderPasses::RenderPassId renderPassId)
{
#if defined(USE_STD_LRU_IMPL)
    if (g_rpCache.find(renderPassId) != g_rpCache.end())
    {
        std::unique_ptr<RenderPassLiveObject> obj = std::move((*g_rpCache[renderPassId]));
        g_rpList.erase(g_rpCache[renderPassId]);
        obj->age = g_tick;

        g_rpList.push_front(std::move(obj));
        g_rpCache[renderPassId] = g_rpList.begin();
        return (*g_rpCache[renderPassId])->rp();
    }
#else
    std::unique_ptr<RenderPassLiveObject>* obj = g_rpCache.refer(renderPassId);
    if (obj)
        return obj->get()->rp();
#endif
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
#if defined(USE_STD_LRU_IMPL)
    auto it = g_fbCache.find(fboId);
    if (it == g_fbCache.end())
        return false;
    return true;
#else
    return g_fbCache.inCache(fboId);
#endif
}


R_INTERNAL
FramebufferObject* getCachedFbo(Hash64 fboId)
{
#if defined(USE_STD_LRU_IMPL)
    if (g_fbCache.find(fboId) != g_fbCache.end())
    {
        std::unique_ptr<FramebufferObject> obj = std::move(*g_fbCache[fboId]);
        g_fbList.erase(g_fbCache[fboId]);
        obj->age = g_tick;
        g_fbList.push_front(std::move(obj));
        g_fbCache[fboId] = g_fbList.begin();
        return g_fbCache[fboId]->get();
    }
#else
    std::unique_ptr<FramebufferObject>* ptr = g_fbCache.refer(fboId);
    if (ptr) return ptr->get();
#endif
    return nullptr;
}


R_INTERNAL
FramebufferObject* cacheFbo(Hash64 fboId, VkFramebuffer fbo, const VkRect2D& renderArea, U32 numReferences)
{
    std::unique_ptr<FramebufferObject> data = std::make_unique<FramebufferObject>();
    data->framebuffer = fbo;
    data->renderArea = renderArea;
#if defined(USE_STD_LRU_IMPL)
    data->id = fboId;
    data->age = g_tick;
    g_fbList.push_front(std::move(data));
    g_fbCache.insert(std::make_pair(fboId, g_fbList.begin()));
    return g_fbCache[fboId]->get();
#else
    return g_fbCache.insert(fboId, std::move(data))->get();
#endif
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
#if defined(USE_STD_LRU_IMPL)
    auto iter = g_rpCache.find(renderPassId);
    // Failed to find a suitable render pass, make a new one.
    if (iter == g_rpCache.end())
#else
    if (!g_rpCache.inCache(renderPassId))
#endif
    {
        VkRenderPass renderPass = createRenderPass(pDevice, desc);
        U32 references = desc.numRenderTargets + ((desc.pDepthStencil != 0) ? 1 : 0);
        cacheRenderPass(renderPassId, renderPass);
#if defined(USE_STD_LRU_IMPL)
        (*g_rpCache[renderPassId])->rp.addReference(references);
        return (*g_rpCache[renderPassId])->rp();
#else
        return g_rpCache.refer(renderPassId)->get()->rp();
#endif
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
#if defined(USE_STD_LRU_IMPL)
    const U32 currentTick = g_tick;

    // Check Render Pass cache.
    if (!g_rpList.empty())
    {
        RenderPassLiveObject* obj = g_rpList.back().get();
        const U32 ageRate = g_tick - obj->age;
        if (ageRate >= g_renderPassMaxAge)
        {
            R_DEBUG("Vulkan", "Destroy Render Pass.");
            g_rpList.pop_back();
            g_rpCache.erase(obj->id);
            VkRenderPass rp = obj->rp();
            vkDestroyRenderPass(device, rp, nullptr);
            // the render pass live object is then cleaned up since it is managed ptr.
        }
    }

    // Now check fbo cache.
    if (!g_fbList.empty())
    {
        FramebufferObject* obj = g_fbList.back().get();
        const U32 ageRate = g_tick - obj->age;
        if (ageRate >= g_renderPassMaxAge)
        {
            R_DEBUG("Vulkan", "Cleaning up framebuffer.");
            g_fbList.pop_back();
            g_fbCache.erase(obj->id);
            VkFramebuffer fbo = obj->framebuffer;
            vkDestroyFramebuffer(device, fbo, nullptr);
        }
    }
#else
    g_rpCache.check(g_renderPassMaxAge,
                        [device] (std::unique_ptr<RenderPassLiveObject>& obj) -> void
                        {
                            R_DEBUG("Vulkan", "Cleaning up render pass");
                            vkDestroyRenderPass(device, obj->rp(), nullptr);
                        });
    g_fbCache.check(g_frameBufferMaxAge, 
                        [device] (std::unique_ptr<FramebufferObject>& obj) -> void 
                        {
                            R_DEBUG("Vulkan", "Cleaning up framebuffer."); 
                            vkDestroyFramebuffer(device, obj->framebuffer, nullptr); 
                        });
#endif
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
#if defined(USE_STD_LRU_IMPL)
    for (auto& iter : g_rpList)
    {
        while (iter.get()->rp.release());
        vkDestroyRenderPass(pDevice->get(), iter->rp(), nullptr);
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying render pass...");
    }
    for (auto& iter : g_fbList)
    {
        vkDestroyFramebuffer(pDevice->get(), iter->framebuffer, nullptr);
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying framebuffer");
    }
    g_fbList.clear();
    g_fbCache.clear();
    g_rpList.clear();
    g_rpCache.clear();
#else
    g_rpCache.forEach
        (
            [pDevice] (std::unique_ptr<RenderPassLiveObject>& obj) -> void
            {
                R_DEBUG(R_CHANNEL_VULKAN, "Destorying RenderPass");
                vkDestroyRenderPass(pDevice->get(), obj->rp(), nullptr);
            }
        );
    g_fbCache.forEach
        (
            [pDevice] (std::unique_ptr<FramebufferObject>& obj) -> void 
            { 
                R_DEBUG(R_CHANNEL_VULKAN, "Destroying framebuffer");
                vkDestroyFramebuffer(pDevice->get(), obj->framebuffer, nullptr); 
            }
        );
    g_rpCache.clear();
    g_fbCache.clear();
#endif
}


void updateTick()
{
#if defined(USE_STD_LRU_IMPL)
    g_tick += 1;
#else
    g_rpCache.updateTick();
    g_fbCache.updateTick();
#endif
}
} // RenderPass
} // Recluse