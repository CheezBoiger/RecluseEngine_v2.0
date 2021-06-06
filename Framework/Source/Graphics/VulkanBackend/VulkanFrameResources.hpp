//
#pragma once

#include "Core/Arch.hpp"
#include "VulkanCommons.hpp"
#include <vector>

namespace Recluse {


class VulkanFrameResources {
public:


    
private:
    std::vector<VkImage> m_frames;
    std::vector<VkImageView> m_frameViews;
    std::vector<VkFramebuffer> m_frameBuffers;
    std::vector<VkSemaphore> m_frameWaitSemaphores;
    std::vector<VkSemaphore> m_frameSignalSemaphores;
    
};
} // Recluse