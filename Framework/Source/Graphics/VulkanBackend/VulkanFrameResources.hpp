//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"

#include <vector>

namespace Recluse {


class VulkanSwapchain;


class VulkanFrameResources 
{
public:

    void                build(VulkanSwapchain* pSwapchain);

    void                destroy(VulkanSwapchain* pSwapchain);

    U32                 getNumMaxFrames() const { return (U32)m_frames.size(); }

    VkImage             getImage(U32 idx) const { return m_frames[idx]; }
    VkFramebuffer       getFrameBuffer(U32 idx) const { return m_frameBuffers[idx]; }
    VkSemaphore         getWaitSemaphore(U32 idx) const { return m_frameWaitSemaphores[idx]; }
    VkSemaphore         getSignalSemaphore(U32 idx) const { return m_frameSignalSemaphores[idx]; }
    VkFence             getFrence(U32 idx) const { return m_frameFences[idx]; }
    
private:

    std::vector<VkImage> m_frames;
    std::vector<VkFramebuffer> m_frameBuffers;
    std::vector<VkSemaphore> m_frameWaitSemaphores;
    std::vector<VkSemaphore> m_frameSignalSemaphores;
    std::vector<VkFence> m_frameFences;
    
};
} // Recluse