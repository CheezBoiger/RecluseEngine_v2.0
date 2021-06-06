//
#pragma once
#include "VulkanCommons.hpp"
#include "Graphics/GraphicsDevice.hpp"

namespace Recluse {


class VulkanSwapchain : public GraphicsSwapchain {
public:
    
    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    U32 build(VkDevice device, U32 desiredFrames, U32 renderWidth, U32 renderHeight);
    
    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

private:

    void buildFrameResources();

    VkSwapchainKHR m_swapchain;
};
} // Recluse