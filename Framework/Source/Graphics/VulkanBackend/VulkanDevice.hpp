// 
#pragma once

#include "VulkanContext.hpp"
#include "Graphics/GraphicsDevice.hpp"

#include <vector>

namespace Recluse {


class VulkanAdapter;
struct DeviceCreateInfo;


class VulkanDevice : public GraphicsDevice {
public:
    VulkanDevice()
        : m_device(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_windowHandle(nullptr) { }

    ErrType initialize(const VulkanAdapter& iadapter, DeviceCreateInfo* info);

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, GraphicsContext* pContext, 
        const SwapchainCreateDescription* pDesc) override;

    ErrType destroySwapchain(GraphicsContext* pContext, GraphicsSwapchain* pSwapchain) override;

    void destroy(VkInstance instance);

    VkDevice operator()() {
        return m_device;
    }

    VkDevice get() const {
        return m_device;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

private:

    ErrType createSurface(VkInstance instance, void* handle);

    VkDevice m_device;
    VkSurfaceKHR m_surface;
    void* m_windowHandle;
};
} // Recluse