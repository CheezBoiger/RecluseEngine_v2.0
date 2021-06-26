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
        , m_windowHandle(nullptr)
        , m_deviceBufferMemory(VK_NULL_HANDLE)
        , m_hostBufferMemory(VK_NULL_HANDLE) { }

    ErrType initialize(VulkanAdapter* iadapter, DeviceCreateInfo* info);

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, 
        const SwapchainCreateDescription* pDesc) override;

    ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    void destroy(VkInstance instance);

    VkDevice operator()() {
        return m_device;
    }

    VkDevice get() const {
        return m_device;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    VulkanAdapter* getAdapter() const { return m_adapter; }

private:

    ErrType createSurface(VkInstance instance, void* handle);

    VulkanAdapter* m_adapter;

    VkDevice m_device;
    VkSurfaceKHR m_surface;
    
    VkDeviceMemory m_hostBufferMemory;
    VkDeviceMemory m_hostTextureMemory;
    VkDeviceMemory m_deviceBufferMemory;
    VkDeviceMemory m_deviceTextureMemory;
    VkDeviceMemory m_uploadMemory;
    VkDeviceMemory m_readbackMemory;

    void* m_windowHandle;
};
} // Recluse