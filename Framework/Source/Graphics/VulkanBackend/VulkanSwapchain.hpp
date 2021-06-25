//
#pragma once
#include "VulkanCommons.hpp"
#include "Graphics/GraphicsDevice.hpp"

namespace Recluse {


class VulkanContext;
class VulkanDevice;

class VulkanSwapchain : public GraphicsSwapchain {
public:
    VulkanSwapchain() 
        : m_swapchain(VK_NULL_HANDLE) { }

    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    ErrType build(VulkanDevice* pDevice, const SwapchainCreateDescription* pDesc);

    ErrType rebuild(const GraphicsContext* pContext, const GraphicsDevice* pDevice, const SwapchainCreateDescription* pDesc) override;
    
    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    // Destroy the vulkan swapchain, including surface.
    ErrType destroy(VkInstance instance, VkDevice device);

private:

    void buildFrameResources();


    VkSwapchainKHR  m_swapchain;
};
} // Recluse