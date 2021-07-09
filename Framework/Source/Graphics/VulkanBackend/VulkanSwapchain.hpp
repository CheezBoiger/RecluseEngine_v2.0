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

struct QueueFamily;

class VulkanSwapchain : public GraphicsSwapchain {
public:
    VulkanSwapchain(const SwapchainCreateDescription& desc) 
        : GraphicsSwapchain(desc)
        , m_swapchain(VK_NULL_HANDLE)
        , m_currentFrameIndex(0)
        , m_currentImageIndex(0)
        , m_pBackbufferQueue(nullptr)
        , m_pDevice(nullptr) { }

    // Build the vulkan swapchain. This will return the total number of frames that 
    // were created.
    ErrType build(VulkanDevice* pDevice);

    ErrType onRebuild() override;
    
    ErrType present() override; 

    U32 getCurrentFrameIndex() override { return m_currentImageIndex; }

    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;
    
    VkSwapchainKHR operator()() {
        return m_swapchain;
    }

    VkSwapchainKHR get() const { return m_swapchain; }

    // Destroy the vulkan swapchain, including surface.
    ErrType destroy();

    VulkanDevice* getDevice() const { return m_pDevice; }

    VkFramebuffer getFramebuffer(U32 idx) const { return m_rawFrames.getFrameBuffer(idx); }
    VkSemaphore getWaitSemaphore(U32 idx) const { return m_rawFrames.getWaitSemaphore(idx); }
    VkSemaphore getSignalSemaphore(U32 idx) const { return m_rawFrames.getSignalSemaphore(idx); }

    VulkanQueue* getPresentationQueue() const { return m_pBackbufferQueue; }

private:

    void buildFrameResources();
    void queryCommandPools();
    void transitionCurrentFrameToPresentable();
    inline void incrementFrameIndex() 
        { m_currentFrameIndex = (m_currentFrameIndex + 1) % m_rawFrames.getNumMaxFrames(); }

    VkSwapchainKHR          m_swapchain;
    VulkanQueue*            m_pBackbufferQueue;
    VulkanDevice*           m_pDevice;
    U32                     m_currentFrameIndex;
    U32                     m_currentImageIndex;
    VulkanFrameResources    m_rawFrames;
    std::vector<VulkanImage*> m_frameResources;
    std::vector<VulkanResourceView*>    m_frameViews;
    std::vector<VkCommandBuffer>  m_commandbuffers;
    const QueueFamily*                  m_queueFamily;
    
};
} // Recluse