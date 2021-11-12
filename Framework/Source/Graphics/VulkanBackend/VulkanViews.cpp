//
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType VulkanResourceView::initialize(VulkanDevice* pDevice)
{
    ErrType result          = REC_RESULT_OK;
    ResourceViewDesc desc   = getDesc();

    VkImageViewCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.format                                 = Vulkan::getVulkanFormat(desc.format);
    info.image                                  = static_cast<VulkanImage*>(desc.pResource)->get();
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

    switch (desc.dimension) {
    
        case RESOURCE_VIEW_DIMENSION_1D: info.viewType = VK_IMAGE_VIEW_TYPE_1D; break;
        case RESOURCE_VIEW_DIMENSION_1D_ARRAY: info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case RESOURCE_VIEW_DIMENSION_2D: info.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
        case RESOURCE_VIEW_DIMENSION_2D_ARRAY: info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case RESOURCE_VIEW_DIMENSION_2D_MS: info.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
        case RESOURCE_VIEW_DIMENSION_3D: info.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        case RESOURCE_VIEW_DIMENSION_CUBE: info.viewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
        case RESOURCE_VIEW_DIMENSION_CUBE_ARRAY: info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; break;
        default: break;
    
    }

    if (desc.type == RESOURCE_VIEW_TYPE_DEPTH_STENCIL) {

        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    } else {
    
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    
    }

    VkResult vResult = vkCreateImageView(pDevice->get(), &info, nullptr, &m_view);

    switch (desc.type) {
        case RESOURCE_VIEW_TYPE_DEPTH_STENCIL: m_expectedLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL; break;
        case RESOURCE_VIEW_TYPE_RENDER_TARGET: m_expectedLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; break;
        case RESOURCE_VIEW_TYPE_SHADER_RESOURCE: m_expectedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
        case RESOURCE_VIEW_TYPE_STORAGE_IMAGE:
        case RESOURCE_VIEW_TYPE_STORAGE_BUFFER: m_expectedLayout = VK_IMAGE_LAYOUT_GENERAL; break;
        default: break;
    }

    m_subresourceRange = info.subresourceRange;
    
    return result;
}


ErrType VulkanResourceView::destroy(VulkanDevice* pDevice)
{
    ErrType result = REC_RESULT_OK;

    if (m_view) {

        R_DEBUG(R_CHANNEL_VULKAN, "Destroying resource view...");
    
        vkDestroyImageView(pDevice->get(), m_view, nullptr);
        
        m_view = VK_NULL_HANDLE;
    
    }

    return result;
}


static VkSamplerAddressMode getAddressMode(SamplerAddressMode mode)
{
    switch (mode) {
        case SAMPLER_ADDRESS_MODE_REPEAT:
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        case SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
}


static VkFilter getFilter(Filter filter)
{
    switch (filter) {
        case FILTER_LINEAR:
        default:
            return VK_FILTER_LINEAR;
        case FILTER_NEAREST:
            return VK_FILTER_NEAREST;
        case FILTER_CUBIC_IMG:
            return VK_FILTER_CUBIC_IMG;
    }
}


static VkSamplerMipmapMode getMipMapMode(SamplerMipMapMode mode)
{
    switch (mode) {
        case SAMPLER_MIP_MAP_MODE_LINEAR:
        default:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case SAMPLER_MIP_MAP_MODE_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}


ErrType VulkanSampler::initialize(VulkanDevice* pDevice, const SamplerCreateDesc& desc)
{
    R_ASSERT(pDevice != NULL);

    VkDevice device             = pDevice->get();
    VkSamplerCreateInfo cInfo   = { };
    VkResult result             = VK_SUCCESS;

    cInfo.sType             = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    cInfo.addressModeU      = getAddressMode(desc.addrModeU);
    cInfo.addressModeV      = getAddressMode(desc.addrModeV);    
    cInfo.addressModeW      = getAddressMode(desc.addrModeW);
    cInfo.anisotropyEnable  = desc.anisotropyEnable;
    cInfo.compareEnable     = desc.compareEnable;
    cInfo.compareOp         = Vulkan::getNativeCompareOp(desc.compareOp);
    cInfo.magFilter         = getFilter(desc.magFilter);
    cInfo.minFilter         = getFilter(desc.minFilter);
    cInfo.minLod            = desc.minLod;
    cInfo.maxLod            = desc.maxLod;
    cInfo.mipmapMode        = getMipMapMode(desc.mipMapMode);
    cInfo.flags             = 0;

    result = vkCreateSampler(device, &cInfo, nullptr, &m_sampler);

    if (result != VK_SUCCESS) {
        return REC_RESULT_FAILED;
    }
    
    return REC_RESULT_OK;
}


ErrType VulkanSampler::destroy(VulkanDevice* pDevice)
{
    R_ASSERT(pDevice != NULL);

    VkDevice device = pDevice->get();
    
    if (m_sampler) {
        vkDestroySampler(device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    return REC_RESULT_OK;
}
} // Recluse