// 
#pragma once

#include "VulkanContext.hpp"
#include "Graphics/GraphicsAdapter.hpp"
#include "Graphics/GraphicsDevice.hpp"

#include <vector>

namespace Recluse {


class VulkanPhysicalDevice : public GraphicsAdapter {
public:

    static std::vector<VulkanPhysicalDevice> getAvailablePhysicalDevices(const VulkanContext& ctx);

    ErrType getAdapterInfo(AdapterInfo* out) const override;

    VkPhysicalDevice operator()() const {
        return m_phyDevice;
    }

    VkPhysicalDevice get() const { 
        return m_phyDevice;
    }

    VkPhysicalDeviceProperties getProperties() const;
    VkPhysicalDeviceMemoryProperties getMemoryProperties() const;
    VkPhysicalDeviceMemoryProperties2 getMemoryProperties2() const;
    VkPhysicalDeviceFeatures getFeatures() const;
    VkPhysicalDeviceFeatures2 getFeatures2() const;

private:
    VkPhysicalDevice m_phyDevice;
};


class VulkanDevice : public GraphicsDevice {
public:

    ErrType initialize(const GraphicsAdapter& iadapter);

    void destroy();

    VkDevice operator()() {
        return m_device;
    }
private:
    VkDevice m_device;
};
} // Recluse