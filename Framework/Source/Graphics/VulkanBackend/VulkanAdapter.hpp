// 
#pragma once
#include "VulkanContext.hpp"
#include "Graphics/GraphicsAdapter.hpp"
#include "Graphics/GraphicsDevice.hpp"

#include <list>

namespace Recluse {


class VulkanDevice;


class VulkanAdapter : public GraphicsAdapter {
public:
    ~VulkanAdapter();

    VulkanAdapter(VulkanAdapter&& a) { 
        m_devices = std::move(a.m_devices); 
        m_phyDevice = a.m_phyDevice; 
        m_context = a.m_context;
    }

    VulkanAdapter& operator=(VulkanAdapter&& a) {
        m_devices = std::move(a.m_devices);
        m_phyDevice = a.m_phyDevice;
        m_context = a.m_context;
        return (*this);
    }

    VulkanAdapter() : m_phyDevice(VK_NULL_HANDLE) { }

    static std::vector<VulkanAdapter> getAvailablePhysicalDevices(VulkanContext* ctx);

    ErrType getAdapterInfo(AdapterInfo* out) const override;

    ErrType createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) override;

    ErrType destroyDevice(GraphicsDevice* pDevice) override;

    U32 VulkanAdapter::findMemoryType(U32 filter, ResourceMemoryUsage usage) const;

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

    std::vector<VkExtensionProperties> getDeviceExtensionProperties() const;

    B32 checkSurfaceSupport(U32 queueIndex, VkSurfaceKHR surface) const;
    
    VulkanContext* getContext() const { return m_context; }

private:

    // Do not allow copying.
    VulkanAdapter(const VulkanAdapter&) = delete;
    VulkanAdapter& operator=(const VulkanAdapter& a) = delete;

    // Native adapter device.
    VkPhysicalDevice m_phyDevice;

    VulkanContext* m_context;

    // All logical devices that were created by this adapter.
    std::list<VulkanDevice*> m_devices;
};
} // Recluse