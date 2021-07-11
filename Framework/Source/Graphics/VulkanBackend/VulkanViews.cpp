//
#include "VulkanViews.hpp"
#include "VulkanResource.hpp"
#include "VulkanDevice.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType VulkanResourceView::initialize(VulkanDevice* pDevice, const ResourceViewDesc& desc)
{
    ErrType result = REC_RESULT_OK;

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
        case RESOURCE_VIEW_TYPE_STORAGE_BUFFER: m_expectedLayout = VK_IMAGE_LAYOUT_GENERAL; break;
        default: break;
    }
    
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
} // Recluse