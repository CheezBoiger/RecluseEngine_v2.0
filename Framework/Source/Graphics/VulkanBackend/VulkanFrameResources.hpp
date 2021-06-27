//
#pragma once

#include "Core/Arch.hpp"
#include "Core/Types.hpp"
#include "VulkanCommons.hpp"

#include <vector>

namespace Recluse {


class VulkanSwapchain;


class VulkanFrameResources {
public:

    void build(VulkanSwapchain* pSwapchain);

    void destroy(VulkanSwapchain* pSwapchain);

    U32 getNumMaxFrames() const { return (U32)m_frames.size(); }
    
private:

    std::vector<VkImage> m_frames;
    std::vector<VkImageView> m_frameViews;
    std::vector<VkFramebuffer> m_frameBuffers;
    std::vector<VkSemaphore> m_frameWaitSemaphores;
    std::vector<VkSemaphore> m_frameSignalSemaphores;
    
};
} // Recluse