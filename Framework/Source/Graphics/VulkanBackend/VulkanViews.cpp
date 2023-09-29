//
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"
#include "VulkanDevice.hpp"
#include "VulkanAdapter.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Vulkan {
namespace ResourceViews {

std::unordered_map<ResourceViewId, VulkanResourceView*>     g_resourceViewMap;
std::unordered_map<SamplerId, VulkanSampler*>               g_samplerMap;


ResourceViewId makeResourceView(VulkanDevice* pDevice, VulkanResource* pResource, const ResourceViewDescription& desc)
{
    
    VulkanResourceView* pView = nullptr;
     
    if (desc.dimension != ResourceViewDimension_Buffer)
        pView = new VulkanImageView(desc);
    else
        pView = new VulkanResourceView(desc, true);
    pView->initialize(pDevice, pResource);
    pView->generateId();
    g_resourceViewMap[pView->getId()] = pView;
    return pView->getId();
}


VulkanSampler* makeSampler(VulkanDevice* pDevice, const SamplerDescription& desc)
{
    VulkanSampler* pSampler         = new VulkanSampler();
    pSampler->initialize(pDevice, desc);
    pSampler->generateId();

    g_samplerMap[pSampler->getId()] = pSampler;
    
    return pSampler;
}


ResultCode releaseResourceView(VulkanDevice* pDevice, ResourceViewId id)
{
    auto& iter = g_resourceViewMap.find(id);
    if (iter == g_resourceViewMap.end())
        return RecluseResult_NotFound;
    iter->second->release(pDevice);
    delete iter->second;
    g_resourceViewMap.erase(iter);
    return RecluseResult_Ok;
}


ResultCode releaseSampler(VulkanDevice* pDevice, SamplerId id)
{
    auto& iter = g_samplerMap.find(id);
    if (iter == g_samplerMap.end())
        return RecluseResult_NotFound;
    iter->second->release(pDevice);
    delete iter->second;
    g_samplerMap.erase(iter);
    return RecluseResult_Ok;
}


VulkanResourceView* obtainResourceView(ResourceViewId id)
{
    auto& iter = g_resourceViewMap.find(id);
    if (iter == g_resourceViewMap.end())
        return nullptr;
    return iter->second;
}


VulkanSampler* obtainSampler(SamplerId id)
{
    auto& iter = g_samplerMap.find(id);
    if (iter == g_samplerMap.end())
        return nullptr;
    return iter->second;
}


void clearCache(VulkanDevice* pDevice)
{
    for (auto iter : g_resourceViewMap)
    {
        if (iter.second)
        {
            iter.second->release(pDevice);
            delete iter.second;
        }
    }

    for (auto iter : g_samplerMap)
    {
        if (iter.second)
        {
            iter.second->release(pDevice);
            delete iter.second;
        }
    }

    g_resourceViewMap.clear();
}
} // ResourceViews

ResourceViewId VulkanResourceView::kResourceViewCreationCounter = 0;
SamplerId VulkanSampler::kSamplerCreationCounter                = 0;

MutexGuard VulkanResourceView::kResourceViewCreationMutex    = MutexGuard("VulkanResourceViewCreationMutex");
MutexGuard VulkanSampler::kSamplerCreationMutex              = MutexGuard("VulkanSamplerCreationMutex");

ResultCode VulkanImageView::onInitialize(VulkanDevice* pDevice, VulkanResource* pResource)
{
    R_ASSERT(pResource != NULL);
    R_ASSERT_FORMAT(!pResource->isBuffer(), "Image resources can not be used as buffer views!");

    ResultCode result          = RecluseResult_Ok;
    ResourceViewDescription desc   = getDesc();

    VkImageViewCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.format                                 = Vulkan::getVulkanFormat(desc.format);
    info.image                                  = static_cast<VulkanImage*>(pResource)->get();
    info.viewType                               = VK_IMAGE_VIEW_TYPE_1D;
    info.subresourceRange.baseArrayLayer        = desc.baseArrayLayer;
    info.subresourceRange.baseMipLevel          = desc.baseMipLevel;
    info.subresourceRange.levelCount            = desc.mipLevelCount;
    info.subresourceRange.layerCount            = desc.layerCount;
    info.flags                                  = 0;
    info.components.r                           = VK_COMPONENT_SWIZZLE_R;
    info.components.g                           = VK_COMPONENT_SWIZZLE_G;
    info.components.b                           = VK_COMPONENT_SWIZZLE_B;
    info.components.a                           = VK_COMPONENT_SWIZZLE_A;

    switch (desc.dimension) 
    {
        case ResourceViewDimension_1d:              info.viewType = VK_IMAGE_VIEW_TYPE_1D; break;
        case ResourceViewDimension_1dArray:         info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case ResourceViewDimension_2d:              info.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
        case ResourceViewDimension_2dArray:         info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case ResourceViewDimension_2dMultisample:   info.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
        case ResourceViewDimension_3d:              info.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        case ResourceViewDimension_Cube:            info.viewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
        case ResourceViewDimension_CubeArray:       info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; break;
        default: break;
    }

    if (desc.type == ResourceViewType_DepthStencil) 
    {
        info.subresourceRange.aspectMask = Vulkan::getDepthStencilAspectFlags(info.format);
    } 
    else 
    {
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkResult vResult = vkCreateImageView(pDevice->get(), &info, nullptr, &m_view);

    switch (desc.type) 
    {
        case ResourceViewType_DepthStencil:     m_expectedLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; break;
        case ResourceViewType_RenderTarget:     m_expectedLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; break;
        case ResourceViewType_ShaderResource:   m_expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
        case ResourceViewType_UnorderedAccess:  m_expectedLayout = VK_IMAGE_LAYOUT_GENERAL; break;
        default: break;
    }

    m_subresourceRange = info.subresourceRange;

    const Bool supportsDebugMarking = pDevice->getAdapter()->getInstance()->supportsDebugMarking();
    const char* debugName           = "VulkanView";
    if (supportsDebugMarking && debugName)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = { };
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
        nameInfo.pNext = nullptr;
        nameInfo.objectHandle = reinterpret_cast<uint64_t>(m_view);
        nameInfo.pObjectName = debugName;
        vResult = pfn_vkSetDebugUtilsObjectNameEXT(pDevice->get(), &nameInfo);
        if (vResult != VK_SUCCESS)
        {
            R_WARN(R_CHANNEL_VULKAN, "Failed to create image view debug name object.");
        }
    }
    
    return result;
}


ResultCode VulkanImageView::onRelease(VulkanDevice* pDevice)
{
    ResultCode result = RecluseResult_Ok;

    if (m_view) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying resource view...");
    
        vkDestroyImageView(pDevice->get(), m_view, nullptr);
        
        m_view = VK_NULL_HANDLE;
    }

    return result;
}


void VulkanResourceView::generateDescriptionId()
{
    DescriptionPacked packed                    = { };
    const ResourceViewDescription& description  = getDesc();
    packed.baseLayer        = description.baseArrayLayer;
    packed.baseMip          = description.baseMipLevel;
    packed.dim              = description.dimension;
    packed.layerCount       = description.layerCount;
    packed.mipCount         = description.mipLevelCount;
    packed.format           = description.format;
    m_descId                = packed.hash0;
}


static VkSamplerAddressMode getAddressMode(SamplerAddressMode mode)
{
    switch (mode) 
    {
        case SamplerAddressMode_Repeat:
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode_MirrorClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        case SamplerAddressMode_ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerAddressMode_ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode_MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
}


static VkFilter getFilter(Filter filter)
{
    switch (filter) 
    {
        case Filter_Linear:
        default:
            return VK_FILTER_LINEAR;
        case Filter_Nearest:
            return VK_FILTER_NEAREST;
        case Filter_Cubic:
            return VK_FILTER_CUBIC_IMG;
    }
}


static VkSamplerMipmapMode getMipMapMode(SamplerMipMapMode mode)
{
    switch (mode) 
    {
        case SamplerMipMapMode_Linear:
        default:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case SamplerMipMapMode_Nearest:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}


R_INTERNAL VkBorderColor getNativeBorderColor(BorderColor borderColor)
{
    switch (borderColor)
    {
        case BorderColor_OpaqueBlack:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case BorderColor_OpaqueWhite:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        case BorderColor_TransparentBlack:
        default:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    }
}


ResultCode VulkanSampler::initialize(VulkanDevice* pDevice, const SamplerDescription& desc)
{
    R_ASSERT(pDevice != NULL);

    VkDevice device             = pDevice->get();
    VkSamplerCreateInfo cInfo   = { };
    VkResult result             = VK_SUCCESS;

    cInfo.sType             = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    cInfo.addressModeU      = getAddressMode(desc.addressModeU);
    cInfo.addressModeV      = getAddressMode(desc.addressModeV);    
    cInfo.addressModeW      = getAddressMode(desc.addressModeW);
    cInfo.anisotropyEnable  = (desc.maxAnisotropy > 0.f) ? true : false;
    cInfo.maxAnisotropy     = desc.maxAnisotropy;
    cInfo.compareEnable     = (desc.compareOp != CompareOp_Never) ? true : false;
    cInfo.compareOp         = Vulkan::getNativeCompareOp(desc.compareOp);
    cInfo.magFilter         = getFilter(desc.magFilter);
    cInfo.minFilter         = getFilter(desc.minFilter);
    cInfo.minLod            = desc.minLod;
    cInfo.maxLod            = desc.maxLod;
    cInfo.mipLodBias        = desc.mipLodBias;
    cInfo.mipmapMode        = getMipMapMode(desc.mipMapMode);
    cInfo.borderColor       = getNativeBorderColor(desc.borderColor);
    cInfo.flags             = 0;

    result = vkCreateSampler(device, &cInfo, nullptr, &m_sampler);

    if (result != VK_SUCCESS) 
    {
        return RecluseResult_Failed;
    }

    generateDescriptionId(desc);
    
    return RecluseResult_Ok;
}


ResultCode VulkanSampler::release(VulkanDevice* pDevice)
{
    R_ASSERT(pDevice != NULL);

    VkDevice device = pDevice->get();
    
    if (m_sampler) 
    {
        vkDestroySampler(device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    return RecluseResult_Ok;
}


void VulkanSampler::generateDescriptionId(const SamplerDescription& description)
{
    SamplerDescriptionUnion obj = { };
    obj.addrU = description.addressModeU;
    obj.addrV = description.addressModeV;
    obj.addrW = description.addressModeW;
    obj.borderColor = description.borderColor;
    obj.compareOp = description.compareOp;
    obj.maxFilter = description.magFilter;
    obj.minFilter = description.minFilter;
    obj.mipMapMode = description.mipMapMode;
    obj.maxAnisotropy = description.maxAnisotropy;
    obj.minLod = description.minLod;
    obj.maxLod = description.maxLod;
    obj.mipLodBias = description.mipLodBias;
    m_descId = recluseHashFast(&obj, sizeof(SamplerDescriptionUnion));
}
} // Vulkan
} // Recluse