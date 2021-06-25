//
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanSwapchain.hpp"
#include "Core/Messaging.hpp"

#include "Graphics/GraphicsAdapter.hpp"

namespace Recluse {


std::vector<const char*> getDeviceExtensions() 
{
    return { };    
}


void checkAvailableDeviceExtensions(const VulkanAdapter& adapter, std::vector<const char*>& extensions)
{
    std::vector<VkExtensionProperties> deviceExtensions = adapter.getDeviceExtensionProperties();
    
    for (U32 i = 0; i < extensions.size(); ++i) {
        B32 found = false;
        for (U32 j = 0; j < deviceExtensions.size(); ++j) { 
    
            if (strcmp(deviceExtensions[j].extensionName, extensions[i]) == 0) {

                R_DEBUG(R_CHANNEL_VULKAN, "Found %s Spec Version: %d", deviceExtensions[j].extensionName,
                    deviceExtensions[j].specVersion);

                found = true;
                break;
            }
    
        }

        if (!found) {

            R_WARN(R_CHANNEL_VULKAN, "%s not found. Removing extension.", extensions[i]);

            extensions.erase(extensions.begin() + i);
            --i;

        }
        
    }
}


ErrType VulkanDevice::initialize(const VulkanAdapter& adapter, DeviceCreateInfo* info)
{
    const VulkanAdapter& nativeAdapter = static_cast<const VulkanAdapter&>(adapter);
    const VulkanContext* pVc = static_cast<const VulkanContext*>(info->pContext);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 
    VkDeviceCreateInfo createInfo = { };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    std::vector<VkQueueFamilyProperties> queueFamilies = nativeAdapter.getQueueFamilyProperties();
    std::vector<const char*> deviceExtensions = getDeviceExtensions();

    if (info->winHandle) {
        R_DEBUG(R_CHANNEL_VULKAN, "Creating surface handle.");

        // Create surface
        ErrType builtSurface = createSurface(pVc->get(), info->winHandle);

        if (builtSurface != REC_RESULT_OK) {

            R_ERR(R_CHANNEL_VULKAN, "Surface failed to create! Aborting build");

            return builtSurface;

        }
 
        m_windowHandle              = info->winHandle;

        R_DEBUG(R_CHANNEL_VULKAN, "Successfully created vulkan handle.");
        
        // Add swapchain extension capability.
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    checkAvailableDeviceExtensions(adapter, deviceExtensions);

    // we just need one priority bit, since we are only allocating one queue for both graphics and compute. 
    F32 priority = 1.0f;

    for (U32 i = 0; i < queueFamilies.size(); ++i) {
        VkDeviceQueueCreateInfo queueInfo       = { };
        VkQueueFamilyProperties queueFamProps   = queueFamilies[i];
        B32 shouldCreateQueues = false;

        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        if (m_surface) {
        
            if (nativeAdapter.checkSurfaceSupport(i, m_surface)) {
            
                R_DEBUG(R_CHANNEL_VULKAN, "Device supports present...");

                queueInfo.queueCount = 1;
                queueInfo.queueFamilyIndex = i;
                queueInfo.pQueuePriorities = &priority;
                shouldCreateQueues = true;
            
            }
        
        }

        if (queueFamProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) {

            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            shouldCreateQueues = true;

        } else if (queueFamProps.queueFlags & VK_QUEUE_COMPUTE_BIT) {

            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            shouldCreateQueues = true;

        } else if (queueFamProps.queueFlags & VK_QUEUE_TRANSFER_BIT) {
        
            queueInfo.queueFamilyIndex = i;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            shouldCreateQueues = true;
        
        }

        if (shouldCreateQueues) {

            queueCreateInfos.push_back(queueInfo);

        }
        
    }
    
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = (U32)queueCreateInfos.size();
    createInfo.enabledExtensionCount = (U32)deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result = vkCreateDevice(nativeAdapter(), &createInfo, nullptr, &m_device);

    if (result != VK_SUCCESS) {

        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan device!");
        
        return -1;
    }
    
    return 0;
}


void VulkanDevice::destroy(VkInstance instance)
{
    if (m_surface) {
    
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed surface.");
    
    }

    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Device Destroyed.");

    }
}


ErrType VulkanDevice::createSwapchain(GraphicsSwapchain** ppSwapchain, GraphicsContext* pContext,
    const SwapchainCreateDescription* pDesc)
{

    VulkanSwapchain* pSwapchain     = new VulkanSwapchain();
    VulkanContext*   pNativeContext = static_cast<VulkanContext*>(pContext);

    ErrType result = pSwapchain->build(this, pDesc);

    if (result != 0) {
        
        R_ERR(R_CHANNEL_VULKAN, "Swapchain failed to create");

        return -1;
    }

    *ppSwapchain = pSwapchain;

    return result;
}


ErrType VulkanDevice::destroySwapchain(GraphicsContext* pContext, GraphicsSwapchain* pSwapchain)
{
    ErrType result = REC_RESULT_OK;

    if (!pSwapchain || !pContext) {

        R_ERR(R_CHANNEL_VULKAN, "Null pointer exception with either pContext or pSwapchain.");
        
        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    VulkanSwapchain* pVs    = static_cast<VulkanSwapchain*>(pSwapchain);
    VulkanContext* pVc      = static_cast<VulkanContext*>(pContext);
    
    pVs->destroy(pVc->get(), m_device);

    return REC_RESULT_OK;
}


ErrType VulkanDevice::createSurface(VkInstance instance, void* handle)
{
    VkResult result             = VK_SUCCESS;

    if (m_surface && (handle == m_windowHandle)) {
    
        R_DEBUG(R_CHANNEL_VULKAN, "Surface is already created for handle. Skipping surface create...");
        
        return REC_RESULT_OK;    
    
    }

    if (!handle) {

        R_ERR(R_CHANNEL_VULKAN, "Null window handle for surface creation.");

        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    if (m_surface) {

        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;

    }

#if defined(RECLUSE_WINDOWS)
    VkWin32SurfaceCreateInfoKHR createInfo = { };

    createInfo.sType            = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance        = GetModuleHandle(NULL);
    createInfo.hwnd             = (HWND)handle;
    createInfo.pNext            = nullptr;
    createInfo.flags            = 0;            // future use, not needed.

    result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_surface);
    
    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create win32 surface.");

        return REC_RESULT_FAILED;
    
    }
#endif

    return REC_RESULT_OK;    
}
} // Recluse