//
#pragma once
#include "VulkanCommons.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "VulkanFrameResources.hpp"

namespace Recluse {


class VulkanContext;
class VulkanDevice;
class VulkanQueue;
class VulkanImage;
class VulkanResourceView;

struct QueueFamily;

// Swapchain handles the swap frames for the display engine of the graphics RHI. 
// Keep in mind this is dependent on buffers available for the graphics engine.
// We can theoretically have as many frames as we may want (although we probably should only have about 2 at least, 3+ recommended)
// but the number of available buffers in the graphics engine is the one that we are governed by.
// 
class VulkanSwapchain : public GraphicsSwapchain 
{
public:
    VulkanSwapchain(const SwapchainCreateDescription& desc, VulkanQueue* pBackbufferQueue) 
        : GraphicsSwapchain(desc)
        , m_swapchain(VK_NULL_HANDLE)
        , m_currentFrameIndex(0)
        , m_currentImageIndex(0)
        , m_pBackbufferQueue(pBackbufferQueue)
        , m_pDevice(nullptr)
        , m_queueFamily(nullptr)
        , m_windowHandle(nullptr) { }

    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    ResultCode                  build(VulkanDevice* pDevice, void* windowHandle);
    ResultCode                  onRebuild() override;
    ResultCode                  present(GraphicsContext* context) override; 
    U32                         getCurrentFrameIndex() override { return m_currentImageIndex; }
    U32                         getFrameCount() const { return m_frames.size(); }
    GraphicsResource*           getFrame(U32 idx) override;

    // Call this function before the start of a primary command buffer record!
    // Prepares the frame for the next image to draw onto.
    // Pass a manual override of a fence object, if we need to wait for another fence,
    // this will override the frame fence wait, and can potentially cause a hang if we lose that
    // fence!
    ResultCode                 prepare(GraphicsContext* context) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    VkSwapchainKHR              get() const { return m_swapchain; }

    // Release the swapchain resources, frames, etc... This does not destroy the surface.
    ResultCode                  release();

    VulkanDevice*               getDevice() const { return m_pDevice; }

    VulkanQueue*                getPresentationQueue() const { return m_pBackbufferQueue; }
    VkPresentModeKHR            checkAvailablePresentMode(VkSurfaceKHR surface, VkPresentModeKHR presentMode);
    void                        validateSwapchainImageIsPresentable();

private:

    void                        buildFrameResources(ResourceFormat resourceFormat);
    inline void                 incrementFrameIndex() 
        { m_currentFrameIndex = (m_currentFrameIndex + 1) % m_frames.size(); }

    VkSwapchainKHR                      m_swapchain;
    VulkanQueue*                        m_pBackbufferQueue;
    VulkanDevice*                       m_pDevice;
    U32                                 m_currentFrameIndex;

    // This is the native image index that is provided from the vulkan API.
    // Use m_currentFrameIndex to find the application frame index instead!
    U32                                 m_currentImageIndex;
    std::vector<VkImage>                m_frames;
    std::vector<VulkanImage*>           m_frameImages;
    //VkCommandPool                     m_commandPool;
    const QueueFamily*                  m_queueFamily;
    void*                               m_windowHandle;
};
} // Recluse