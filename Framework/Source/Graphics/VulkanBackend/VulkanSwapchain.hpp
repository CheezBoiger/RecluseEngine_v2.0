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
        , m_queueFamily(nullptr) { }

    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    ErrType                 build(VulkanDevice* pDevice);

    ErrType                 onRebuild() override;
    
    ErrType                 present(PresentConfig config = PresentConfig_Present) override; 

    U32                     getCurrentFrameIndex() override { return m_currentImageIndex; }

    GraphicsResource*       getFrame(U32 idx) override;
    GraphicsResourceView*   getFrameView(U32 idx) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    VkSwapchainKHR  get() const { return m_swapchain; }

    // Destroy the vulkan swapchain, including surface.
    ErrType         destroy();

    VulkanDevice*   getDevice() const { return m_pDevice; }

    VkFramebuffer   getFramebuffer(U32 idx) const { return m_frameResources.getFrameBuffer(idx); }
    VkSemaphore     getWaitSemaphore(U32 idx) const { return m_frameResources.getWaitSemaphore(idx); }
    VkSemaphore     getSignalSemaphore(U32 idx) const { return m_frameResources.getSignalSemaphore(idx); }
    VkFence         getFence(U32 idx) const { return m_frameResources.getFence(idx); }

    VulkanQueue*    getPresentationQueue() const { return m_pBackbufferQueue; }

    // Submit the final command buffer, which will utilize the wait and signal semphores for the current frame.
    ErrType         submitFinalCommandBuffer(VkCommandBuffer commandBuffer, VkFence cpuFence);

private:

    void        buildFrameResources(ResourceFormat resourceFormat);
    void        queryCommandPools();
    inline void incrementFrameIndex() 
        { m_currentFrameIndex = (m_currentFrameIndex + 1) % m_frameResources.getNumMaxFrames(); }

    VkSwapchainKHR                      m_swapchain;
    VulkanQueue*                        m_pBackbufferQueue;
    VulkanDevice*                       m_pDevice;
    U32                                 m_currentFrameIndex;
    U32                                 m_currentImageIndex;
    VulkanFrameResources                m_frameResources;
    std::vector<VulkanImage*>           m_frameImages;
    std::vector<VulkanResourceView*>    m_frameViews;
    std::vector<VkCommandBuffer>        m_commandbuffers;
    VkCommandPool                       m_commandPool;
    const QueueFamily*                  m_queueFamily;
    
};
} // Recluse