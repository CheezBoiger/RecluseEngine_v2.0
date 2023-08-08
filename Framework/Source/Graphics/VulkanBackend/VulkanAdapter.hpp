// 
#pragma once
#include "VulkanInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include <list>

namespace Recluse {


class VulkanDevice;


class VulkanAdapter : public GraphicsAdapter 
{
public:
    ~VulkanAdapter();

    VulkanAdapter(VulkanAdapter&& a) noexcept
    { 
        m_devices = std::move(a.m_devices); 
        m_supportedDeviceExtensionFlags = a.m_supportedDeviceExtensionFlags;
        m_supportedDeviceExtensions = std::move(a.m_supportedDeviceExtensions);
        m_phyDevice = a.m_phyDevice; 
        m_instance = a.m_instance;
        m_id = a.m_id;
        m_memoryProperties = internalGetPhysicalMemoryProperties();
        m_properties = internalGetPhysicalProperties();
        m_features = internalGetPhysicalFeatures();
    }

    VulkanAdapter& operator=(VulkanAdapter&& a) noexcept
    {
        m_devices = std::move(a.m_devices);
        m_supportedDeviceExtensionFlags = a.m_supportedDeviceExtensionFlags;
        m_supportedDeviceExtensions = std::move(a.m_supportedDeviceExtensions);
        m_phyDevice = a.m_phyDevice;
        m_instance = a.m_instance;
        m_id = a.m_id;
        m_memoryProperties = internalGetPhysicalMemoryProperties();
        m_properties = internalGetPhysicalProperties();
        m_features = internalGetPhysicalFeatures();
        return (*this);
    }

    VulkanAdapter(U32 id = 0) 
        : m_phyDevice(VK_NULL_HANDLE)
        , m_instance(nullptr)
        , m_supportedDeviceExtensionFlags(LayerFeatureFlag_None)
        , m_id(id) 
    { }

    static std::vector<VulkanAdapter>       getAvailablePhysicalDevices(VulkanInstance* ctx);
    static VkDeviceSize                     obtainMinUniformBufferOffsetAlignment(VulkanDevice* pDevice);
    static VkDeviceSize                     obtainMinStorageBufferOffsetAlignment(VulkanDevice* pDevice);
    static VkDeviceSize                     obtainMinMemoryMapAlignment(VulkanDevice* pDevice);

    ResultCode                              getAdapterInfo(AdapterInfo* out) const override;
    ResultCode                              createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) override;
    ResultCode                              destroyDevice(GraphicsDevice* pDevice) override;

    U32                                     findMemoryType(U32 filter, ResourceMemoryUsage usage) const;
    U32                                     constantBufferOffsetAlignmentBytes() const;

    VkPhysicalDevice                        operator()() const { return m_phyDevice; }

    VkPhysicalDevice                        get() const { return m_phyDevice; }

    const VkPhysicalDeviceProperties&       getProperties() const;
    const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const;
    VkPhysicalDeviceMemoryProperties2       getMemoryProperties2() const;
    const VkPhysicalDeviceFeatures&         getFeatures() const;
    VkPhysicalDeviceFeatures2               getFeatures2() const;
    VkFormatProperties                      getFormatProperties(VkFormat format) const;
    std::vector<VkQueueFamilyProperties>    getQueueFamilyProperties() const;
    std::vector<VkExtensionProperties>      getDeviceExtensionProperties() const;
    std::vector<VkPresentModeKHR>           getSupportedPresentModes(VkSurfaceKHR surface) const;

    B32                                     checkSurfaceSupport(U32 queueIndex, VkSurfaceKHR surface) const;
    U32                                     getId() const { return m_id; }
    
    VulkanInstance*                         getInstance() const { return m_instance; }
    std::vector<VkSurfaceFormatKHR>         getSurfaceFormats(VkSurfaceKHR surface);
    std::vector<const char*>                queryAvailableDeviceExtensions(LayerFeatureFlags requested) const;

    Bool                                    checkSupportsExtension(LayerFeatureFlags flags) const { return (m_supportedDeviceExtensionFlags & flags); }
    Bool                                    supportsRayTracing() const { return checkSupportsExtension(LayerFeatureFlag_Raytracing); }

private:

    // Do not allow copying.
    VulkanAdapter(const VulkanAdapter&) = delete;
    VulkanAdapter& operator=(const VulkanAdapter& a) = delete;

    void                                    checkAvailableDeviceExtensions();
    VkPhysicalDeviceMemoryProperties        internalGetPhysicalMemoryProperties();
    VkPhysicalDeviceProperties              internalGetPhysicalProperties();
    VkPhysicalDeviceFeatures                internalGetPhysicalFeatures();

    // Native adapter device.
    VkPhysicalDevice                        m_phyDevice;

    // Context handle.
    VulkanInstance*                         m_instance;
    VkPhysicalDeviceMemoryProperties        m_memoryProperties;
    VkPhysicalDeviceProperties              m_properties;
    VkPhysicalDeviceFeatures                m_features;

    // All logical devices that were created by this adapter.
    std::list<VulkanDevice*>                m_devices;

    // Supported device extension flags, to be used by request of creation of logical devices from this physical device adapter.
    LayerFeatureFlags                       m_supportedDeviceExtensionFlags;

    // This adapter id.
    U32                                     m_id;

    std::vector<std::tuple<LayerFeatureFlag, std::vector<const char*>>> m_supportedDeviceExtensions;
};
} // Recluse