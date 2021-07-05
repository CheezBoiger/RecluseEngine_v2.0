//
#pragma once
#include "VulkanCommons.hpp"
#include "Graphics/GraphicsDevice.hpp"
#include "VulkanFrameResources.hpp"

namespace Recluse {


class VulkanContext;
class VulkanDevice;
class VulkanQueue;
class VulkanImage;
class VulkanResourceView;

class VulkanSwapchain : public GraphicsSwapchain {
public:
    VulkanSwapchain(const SwapchainCreateDescription& desc) 
        : GraphicsSwapchain(desc)
        , m_swapchain(VK_NULL_HANDLE)
        , m_currentFrameIndex(0)
        , m_pBackbufferQueue(nullptr)
        , m_pDevice(nullptr) { }

    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    ErrType build(VulkanDevice* pDevice);

    ErrType onRebuild() override;
    
    ErrType present() override; 

    U32 getCurrentFrameIndex() override { return m_currentFrameIndex; }

    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    VkSwapchainKHR get() const { return m_swapchain; }

    // Destroy the vulkan swapchain, including surface.
    ErrType destroy();

    VulkanDevice* getDevice() const { return m_pDevice; }

private:

    void buildFrameResources();

    VkSwapchainKHR          m_swapchain;
    VulkanQueue*            m_pBackbufferQueue;
    VulkanDevice*           m_pDevice;
    U32                     m_currentFrameIndex;
    VulkanFrameResources    m_rawFrames;
    std::vector<VulkanImage*> m_frameResources;
    std::vector<VulkanResourceView*>    m_frameViews;
    
};
} // Recluse