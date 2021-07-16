//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Graphics/ResourceView.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class VulkanDevice;

class VulkanResourceView : public GraphicsResourceView {
public:
    VulkanResourceView(const ResourceViewDesc& desc)
        : GraphicsResourceView(desc)
        , m_expectedLayout(VK_IMAGE_LAYOUT_UNDEFINED)
        , m_view(VK_NULL_HANDLE)
        , m_subresourceRange({ }) { }

    ErrType initialize(VulkanDevice* pDevice);

    ErrType destroy(VulkanDevice* pDevice);

    VkImageView get() const { return m_view; }

    VkImageLayout getExpectedLayout() const { return m_expectedLayout; }

    VkImageSubresourceRange getSubresourceRange() const { return m_subresourceRange; }

private:
    VkImageView m_view;
    VkImageLayout m_expectedLayout;
    VkImageSubresourceRange m_subresourceRange;
};
} // Recluse