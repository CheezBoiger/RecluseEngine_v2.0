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
    ResultCode                 build(VulkanDevice* pDevice);

    ResultCode                 onRebuild() override;
    
    ResultCode                 present(PresentConfig config = PresentConfig_Present) override; 

    U32                     getCurrentFrameIndex() override { return m_currentFrameIndex; }
    U32                     getFrameCount() const { return m_frameResources.getNumMaxFrames(); }

    GraphicsResource*       getFrame(U32 idx) override;

    // Call this function before the start of a primary command buffer record!
    // Prepares the frame for the next image to draw onto.
    // Pass a manual override of a fence object, if we need to wait for another fence,
    // this will override the frame fence wait, and can potentially cause a hang if we lose that
    // fence!
    ResultCode                 prepareFrame(VkFence cpuFence = VK_NULL_HANDLE);
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    VkSwapchainKHR  get() const { return m_swapchain; }

    // Destroy the vulkan swapchain, including surface.
    ResultCode         destroy();

    VulkanDevice*   getDevice() const { return m_pDevice; }

    VkFramebuffer   getFramebuffer(U32 idx) const { return m_frameResources.getFrameBuffer(idx); }
    VkSemaphore     getWaitSemaphore(U32 idx) const { return m_frameResources.getWaitSemaphore(idx); }
    VkSemaphore     getSignalSemaphore(U32 idx) const { return m_frameResources.getSignalSemaphore(idx); }
    VkFence         getFence(U32 idx) const { return m_frameResources.getFence(idx); }

    VulkanQueue*    getPresentationQueue() const { return m_pBackbufferQueue; }

    // Submit the final command buffer, which will utilize the wait and signal semphores for the current frame.
    // Can optionally override the frame inflight fence with your own fence if needed. Keep in mind this can be 
    // dangerous!
    ResultCode         submitFinalCommandBuffer(VkCommandBuffer commandBuffer, VkFence cpuFence = VK_NULL_HANDLE);

private:

    void        buildFrameResources(ResourceFormat resourceFormat);
    void        queryCommandPools();
    inline void incrementFrameIndex() 
        { m_currentFrameIndex = (m_currentFrameIndex + 1) % m_frameResources.getNumMaxFrames(); }

    VkSwapchainKHR                      m_swapchain;
    VulkanQueue*                        m_pBackbufferQueue;
    VulkanDevice*                       m_pDevice;
    U32                                 m_currentFrameIndex;

    // This is the native image index that is provided from the vulkan API.
    // Use m_currentFrameIndex to find the application frame index instead!
    U32                                 m_currentImageIndex;
    VulkanFrameResources                m_frameResources;
    std::vector<VulkanImage*>           m_frameImages;
    std::vector<VkCommandBuffer>        m_commandbuffers;
    VkCommandPool                       m_commandPool;
    const QueueFamily*                  m_queueFamily;
    
};
} // Recluse