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

    ErrType initialize(const VulkanAdapter& iadapter, DeviceCreateInfo* info);

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, GraphicsContext* pContext, 
        const SwapchainCreateDescription* pDesc) override;

    void destroy();

    VkDevice operator()() {
        return m_device;
    }
private:
    VkDevice m_device;
};
} // Recluse