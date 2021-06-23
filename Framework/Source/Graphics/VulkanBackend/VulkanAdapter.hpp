// 
#pragma once
#include "VulkanContext.hpp"
#include "Graphics/GraphicsAdapter.hpp"

#include <list>

namespace Recluse {


class VulkanDevice;


class VulkanAdapter : public GraphicsAdapter {
public:
    ~VulkanAdapter();

    VulkanAdapter(VulkanAdapter&& a) { 
        m_devices = std::move(a.m_devices); 
        m_phyDevice = a.m_phyDevice; 
    }

    VulkanAdapter& operator=(VulkanAdapter&& a) {
        m_devices = std::move(a.m_devices);
        m_phyDevice = a.m_phyDevice;
        return (*this);
    }

    VulkanAdapter() : m_phyDevice(VK_NULL_HANDLE) { }

    static std::vector<VulkanAdapter> getAvailablePhysicalDevices(const VulkanContext& ctx);

    ErrType getAdapterInfo(AdapterInfo* out) const override;

    ErrType createDevice(DeviceCreateInfo* info, GraphicsDevice** ppDevice) override;

    ErrType destroyDevice(GraphicsDevice* pDevice) override;

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

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;

private:

    // Do not allow copying.
    VulkanAdapter(const VulkanAdapter&) = delete;
    VulkanAdapter& operator=(const VulkanAdapter& a) = delete;

    // Native adapter device.
    VkPhysicalDevice m_phyDevice;

    // All logical devices that were created by this adapter.
    std::list<VulkanDevice*> m_devices;
};
} // Recluse