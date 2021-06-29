//
#pragma once

#include "VulkanCommons.hpp"
#include "Graphics/ResourceView.hpp"
#include "Graphics/GraphicsDevice.hpp"

namespace Recluse {

class VulkanDevice;

class VulkanResourceView : public GraphicsResourceView {
public:

    ErrType initialize(VulkanDevice* pDevice, const ResourceViewDesc& desc);

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