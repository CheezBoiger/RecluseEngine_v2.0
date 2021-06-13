//
#pragma once
#include "VulkanCommons.hpp"
#include "Graphics/GraphicsDevice.hpp"

namespace Recluse {


class VulkanContext;


class VulkanSwapchain : public GraphicsSwapchain {
public:
    
    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    ErrType build(VkDevice device, VulkanContext* pContext, const SwapchainCreateDescription* pDesc);

    ErrType rebuild(const GraphicsContext* pContext, const SwapchainCreateDescription* pDesc) override;
    
    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

private:

    void buildFrameResources();

    ErrType createSurface(VkInstance instance, void* handle);

    VkSwapchainKHR  m_swapchain;
    VkSurfaceKHR    m_surface;
    void*           m_windowHandle;
};
} // Recluse