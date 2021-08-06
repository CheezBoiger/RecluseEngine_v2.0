// 
#pragma once
#include "VulkanInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include <list>

namespace Recluse {


class VulkanDevice;


class VulkanAdapter : public GraphicsAdapter {
public:
    ~VulkanAdapter();

    VulkanAdapter(VulkanAdapter&& a) { 
        m_devices = std::move(a.m_devices); 
        m_phyDevice = a.m_phyDevice; 
        m_instance = a.m_instance;
    }

    VulkanAdapter& operator=(VulkanAdapter&& a) {
        m_devices = std::move(a.m_devices);
        m_phyDevice = a.m_phyDevice;
        m_instance = a.m_instance;
        return (*this);
    }

    VulkanAdapter() : m_phyDevice(VK_NULL_HANDLE), m_instance(nullptr) { }

    static std::vector<VulkanAdapter> getAvailablePhysicalDevices(VulkanInstance* ctx);

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

    VkFormatProperties getFormatProperties(VkFormat format) const;

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;

    std::vector<VkExtensionProperties> getDeviceExtensionProperties() const;

    B32 checkSurfaceSupport(U32 queueIndex, VkSurfaceKHR surface) const;
    
    VulkanInstance* getInstance() const { return m_instance; }

private:

    // Do not allow copying.
    VulkanAdapter(const VulkanAdapter&) = delete;
    VulkanAdapter& operator=(const VulkanAdapter& a) = delete;

    // Native adapter device.
    VkPhysicalDevice m_phyDevice;

    // Context handle.
    VulkanInstance* m_instance;

    // All logical devices that were created by this adapter.
    std::list<VulkanDevice*> m_devices;
};
} // Recluse