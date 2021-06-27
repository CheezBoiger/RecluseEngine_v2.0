//
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanQueue.hpp"
#include "VulkanResource.hpp"
#include "Core/Messaging.hpp"

#include "Graphics/GraphicsAdapter.hpp"

#include "Core/Memory/MemoryPool.hpp"
#include "Core/Memory/Allocator.hpp"

namespace Recluse {


std::vector<const char*> getDeviceExtensions() 
{
    return { };    
}


void checkAvailableDeviceExtensions(const VulkanAdapter* adapter, std::vector<const char*>& extensions)
{
    std::vector<VkExtensionProperties> deviceExtensions = adapter->getDeviceExtensionProperties();
    
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


ErrType VulkanDevice::initialize(VulkanAdapter* adapter, DeviceCreateInfo& info)
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 

    VkDeviceCreateInfo createInfo                       = { };

    VulkanContext* pVc                                  = adapter->getContext();
    std::vector<VkQueueFamilyProperties> queueFamilies  = adapter->getQueueFamilyProperties();
    std::vector<const char*> deviceExtensions           = getDeviceExtensions();

    createInfo.sType                                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    if (info.winHandle) {
        R_DEBUG(R_CHANNEL_VULKAN, "Creating surface handle.");

        // Create surface
        ErrType builtSurface = createSurface(pVc->get(), info.winHandle);

        if (builtSurface != REC_RESULT_OK) {

            R_ERR(R_CHANNEL_VULKAN, "Surface failed to create! Aborting build");

            return builtSurface;

        }
 
        m_windowHandle              = info.winHandle;

        R_DEBUG(R_CHANNEL_VULKAN, "Successfully created vulkan handle.");
        
        // Add swapchain extension capability.
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    checkAvailableDeviceExtensions(adapter, deviceExtensions);

    // we just need one priority bit, since we are only allocating one queue for both graphics and compute. 
    F32 priority = 1.0f;
    std::vector<std::vector<F32>> priorities(queueFamilies.size());

    for (U32 i = 0; i < queueFamilies.size(); ++i) {
        QueueFamily             queueFamily     = { };
        VkDeviceQueueCreateInfo queueInfo       = { };
        VkQueueFamilyProperties queueFamProps   = queueFamilies[i];
        B32 shouldCreateQueues                  = false;

        queueInfo.queueCount    = 0;
        queueInfo.sType         = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        if (m_surface) {
        
            if (adapter->checkSurfaceSupport(i, m_surface)) {
            
                R_DEBUG(R_CHANNEL_VULKAN, "Device supports present...");

                queueInfo.queueCount += 1;
                queueFamily.flags |= QUEUE_TYPE_PRESENT;
                shouldCreateQueues = true;
            
            }
        
        }

        if (queueFamProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) {

            queueInfo.queueCount += 1;
            queueFamily.flags |= QUEUE_TYPE_GRAPHICS;
            shouldCreateQueues = true;

        } 
    
        if (queueFamProps.queueFlags & VK_QUEUE_COMPUTE_BIT) {

            queueInfo.queueCount += 1;
            queueFamily.flags |= QUEUE_TYPE_COMPUTE;
            shouldCreateQueues = true;

        } 

        if (queueFamProps.queueFlags & VK_QUEUE_TRANSFER_BIT) {
        
            queueInfo.queueCount += 1;
            queueFamily.flags |= QUEUE_TYPE_COPY;
            shouldCreateQueues = true;
        
        }

        if (shouldCreateQueues) {

            // Check if the queue count is too big!
            queueInfo.queueCount = 
                ((queueInfo.queueCount > queueFamilies[i].queueCount) ? 
                queueFamilies[i].queueCount : queueInfo.queueCount);
            queueInfo.queueFamilyIndex = i;

            priorities[i].resize(queueInfo.queueCount);

            for (U32 j = 0; j < priorities[i].size(); ++j)
                priorities[i][j] = priority;

            queueInfo.pQueuePriorities = priorities[i].data();

            queueCreateInfos.push_back(queueInfo);

            queueFamily.maxQueueCount       = queueInfo.queueCount;
            queueFamily.queueFamilyIndex    = i;
            m_queueFamilies.push_back(queueFamily);

        }
        
    }
    
    createInfo.pQueueCreateInfos        = queueCreateInfos.data();
    createInfo.queueCreateInfoCount     = (U32)queueCreateInfos.size();
    createInfo.enabledExtensionCount    = (U32)deviceExtensions.size();
    createInfo.ppEnabledExtensionNames  = deviceExtensions.data();

    VkResult result = vkCreateDevice(adapter->get(), &createInfo, nullptr, &m_device);

    if (result != VK_SUCCESS) {

        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan device!");
        
        return -1;
    }
    
    m_adapter = adapter;

    return 0;
}


void VulkanDevice::destroy(VkInstance instance)
{

    if (!m_queues.empty()) {
    
        R_WARN(R_CHANNEL_VULKAN, "One or more queue handles still exist! This can cause memory leaks!");
    
    }

    if (!m_swapchains.empty()) {
    
        R_WARN(R_CHANNEL_VULKAN, "One or more swapchain handles still exist! This can cause memory leaks!");
    
    }

    if (m_surface) {
    
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed surface.");
    
    }

    for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) {
    
        if (m_deviceMemory[i]) {
    
            vkFreeMemory(m_device, m_deviceMemory[i], nullptr);
            m_deviceMemory[i] = nullptr;
    
        }
    
    }

    if (m_device != VK_NULL_HANDLE) {

        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Device Destroyed.");

    }
}


ErrType VulkanDevice::createSwapchain(GraphicsSwapchain** ppSwapchain,
    const SwapchainCreateDescription& pDesc)
{

    VulkanSwapchain* pSwapchain     = new VulkanSwapchain();
    VulkanContext*   pNativeContext = m_adapter->getContext();

    ErrType result = pSwapchain->build(this, pDesc);

    if (result != 0) {
        
        R_ERR(R_CHANNEL_VULKAN, "Swapchain failed to create");

        return -1;
    }

    *ppSwapchain = pSwapchain;

    m_swapchains.push_back(pSwapchain);

    return result;
}


ErrType VulkanDevice::destroySwapchain(GraphicsSwapchain* pSwapchain)
{
    ErrType result = REC_RESULT_OK;

    if (!pSwapchain) {

        R_ERR(R_CHANNEL_VULKAN, "Null pointer exception with either pContext or pSwapchain.");
        
        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    VulkanSwapchain* pVs    = static_cast<VulkanSwapchain*>(pSwapchain);

    for (auto& iter = m_swapchains.begin(); iter != m_swapchains.end(); ++iter) {
    
        if (*iter == pVs) {

            result = pVs->destroy(m_adapter->getContext()->get(), m_device);

            if (result != REC_RESULT_OK) {
    
                R_ERR(R_CHANNEL_VULKAN, "Failed to destroy vulkan swapchain!");
    
            } else {

                R_DEBUG(R_CHANNEL_VULKAN, "Destroyed vulkan swapchain...");
        
                m_swapchains.erase(iter);
                delete pVs;
    
            }

            break;

        }

    }

    return result;
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


ErrType VulkanDevice::reserveMemory(const MemoryReserveDesc& desc)
{
    VkMemoryRequirements memoryRequirements = { };

    R_DEBUG(R_CHANNEL_VULKAN, "Reserving memory for:\n\tHost Buffer Memory (Bytes): \t%llu\n"
        "\tHost Texture Memory (Bytes): \t%llu\n\tDevice Buffer Memory (Bytes): \t%llu\n"
        "\tDevice Texture Memory (Bytes): \t%llu", desc.hostBufferMemoryBytes, desc.hostTextureMemoryBytes,
        desc.deviceBufferMemoryBytes, desc.deviceTextureMemoryBytes);

    R_DEBUG(R_CHANNEL_VULKAN, "Total available memory (GB):\n\tDevice: %f\n\tHost: %f", 
        F32(desc.deviceBufferMemoryBytes + desc.deviceTextureMemoryBytes) / R_1GB,
        F32(desc.hostBufferMemoryBytes + desc.hostTextureMemoryBytes) / R_1GB);

    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // start with memory gpu.
    VkBufferCreateInfo bufferInfo = { };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.deviceBufferMemoryBytes;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer buffer;
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);

    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create temp buffer!");
    
    }

    vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

    allocInfo.memoryTypeIndex = 
        m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, RESOURCE_MEMORY_USAGE_GPU_ONLY);

    allocInfo.allocationSize = RECLUSE_ALLOC_MASK(memoryRequirements.size, memoryRequirements.alignment);

    R_DEBUG(R_CHANNEL_VULKAN, "Allocating device memory...");

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_deviceMemory[RESOURCE_MEMORY_USAGE_GPU_ONLY]);
    
    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate device memory!")
    
    }

    vkDestroyBuffer(m_device, buffer, nullptr);

    // Allocate host memory.

    bufferInfo.size = desc.hostBufferMemoryBytes;

    result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
    vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

    allocInfo.memoryTypeIndex = 
        m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, RESOURCE_MEMORY_USAGE_CPU_ONLY);
    allocInfo.allocationSize = RECLUSE_ALLOC_MASK(memoryRequirements.size, memoryRequirements.alignment);
    
    R_DEBUG(R_CHANNEL_VULKAN, "Allocating host memory...");

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_deviceMemory[RESOURCE_MEMORY_USAGE_CPU_ONLY]);
    
    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate host memory!");

    }

    vkDestroyBuffer(m_device, buffer, nullptr);

    return REC_RESULT_OK;
}


ErrType VulkanDevice::createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type)
{
    U32 queueFamilyIndex    = 0xFFFFFFFF;
    U32 queueIndex          = 0xFFFFFFFF;
    VulkanQueue* pQueue     = nullptr;

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) {

        QueueFamily& family = m_queueFamilies[i];

        if (family.flags & type) {
            
            // Check if we can get a queue from this family.
            if (family.currentAvailableQueueIndex < family.maxQueueCount) {
                
                queueFamilyIndex    = family.queueFamilyIndex;
                queueIndex          = family.currentAvailableQueueIndex++;
                break;
                
            }
           
    
        }
    
    }

    if (queueFamilyIndex == 0xFFFFFFFF || queueIndex == 0xFFFFFFFF) {
    
        R_ERR(R_CHANNEL_VULKAN, "Could not find proper queue family! Can not create queue.");

        return REC_RESULT_FAILED;
    
    }

    pQueue = new VulkanQueue(type);

    ErrType err = pQueue->initialize(m_device, queueFamilyIndex, queueIndex);

    if (err == REC_RESULT_OK) {

        *ppQueue = pQueue;

        m_queues.push_back(pQueue);

    }

    return err;
}


ErrType VulkanDevice::destroyCommandQueue(GraphicsQueue* pQueue)
{
    for (auto& iter = m_queues.begin(); iter != m_queues.end(); ++iter) {
    
        if (*iter == pQueue) {
    
            m_queues.erase(iter);
            delete pQueue;

            return REC_RESULT_OK;
            
        }
    
    }

    R_WARN(R_CHANNEL_VULKAN, "This queue does not exist from this device. Ignoring this destruction function...");
    
    return REC_RESULT_FAILED;
}


ErrType VulkanDevice::createResource(GraphicsResource** ppResource, GraphicsResourceDescription& desc)
{
    ErrType result = REC_RESULT_OK;

    if (desc.dimension == RESOURCE_DIMENSION_BUFFER) {

        VulkanBuffer* pBuffer = new VulkanBuffer(desc);

        result = pBuffer->initialize(this, desc);

        *ppResource = pBuffer;

    } else {
    
        VulkanImage* pImage = new VulkanImage(desc);

        result = pImage->initialize(this, desc);

        *ppResource = pImage;
    
    }
    return result;
}
} // Recluse