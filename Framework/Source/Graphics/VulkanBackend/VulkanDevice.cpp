//
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanQueue.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
#include "VulkanCommandList.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"

#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/StackAllocator.hpp"

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

    // Create the command pool.
    createCommandPools(info.buffering);    
    createFences(info.buffering);
    m_bufferCount = info.buffering;

    return 0;
}


void VulkanDevice::destroy(VkInstance instance)
{
    vkDeviceWaitIdle(m_device);
    destroyCommandPools();
    destroyFences();

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
    
        if (m_bufferPool[i].memory) {
    
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated pool...");

            vkFreeMemory(m_device, m_bufferPool[i].memory, nullptr);
            m_bufferPool[i].memory = nullptr;
    
        }

        if (m_imagePool[i].memory) {
        
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated image pool...");

            vkFreeMemory(m_device, m_imagePool[i].memory, nullptr);
            m_imagePool[i].memory = nullptr;
        
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

    VulkanSwapchain* pSwapchain     = new VulkanSwapchain(pDesc);
    VulkanContext*   pNativeContext = m_adapter->getContext();

    ErrType result = pSwapchain->build(this);

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
            VulkanQueue* pPq = pVs->getPresentationQueue();

            pPq->wait();

            result = pVs->destroy();

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
        "\n\tDevice Buffer Memory (Bytes): \t%llu\n\tDevice Texture Memory (Bytes): \t%llu", 
        desc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY], 
        desc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY], 
        desc.texturePoolGPUOnly);

    R_DEBUG(R_CHANNEL_VULKAN, "Total available memory (GB):\n\tDevice: %f\n\tHost: %f", 
        F32(desc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY] + desc.texturePoolGPUOnly) / R_1GB,
        F32(desc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY]) / R_1GB);

    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VkBuffer buffer;
    VkImage image;
    VkResult result = VK_SUCCESS;

    for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) {

        // start with memory gpu.
        VkBufferCreateInfo bufferInfo = { };
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.bufferPools[i];
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);

        if (result != VK_SUCCESS) {
    
            R_ERR(R_CHANNEL_VULKAN, "Failed to create temp buffer!");
    
        }

        vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

        allocInfo.memoryTypeIndex = 
            m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, (ResourceMemoryUsage)i);

        allocInfo.allocationSize = R_ALLOC_MASK(memoryRequirements.size, memoryRequirements.alignment);

        R_DEBUG(R_CHANNEL_VULKAN, "Allocating device memory...");

        result = vkAllocateMemory(m_device, &allocInfo, nullptr, 
            &m_bufferPool[i].memory);
    
        if (result != VK_SUCCESS) {
    
            R_ERR(R_CHANNEL_VULKAN, "Failed to allocate device memory!")
    
        }

        m_bufferPool[i].sizeBytes = allocInfo.allocationSize;

        if (i != RESOURCE_MEMORY_USAGE_GPU_ONLY) {

            vkMapMemory(m_device, m_bufferPool[i].memory, 0, allocInfo.allocationSize, 0, 
                &m_bufferPool[i].basePtr);

        }

        vkDestroyBuffer(m_device, buffer, nullptr);

        // TODO: Need to add allocator for buffers later.
        m_bufferAllocators[i] = new VulkanAllocator();
        m_bufferAllocators[i]->initialize(new StackAllocator(), 
            &m_bufferPool[i]);
    }
    
    // Create image pools.
#if 1
    // start with memory gpu.
    VkImageCreateInfo imageInfo = { };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.arrayLayers = 1;
    imageInfo.extent.width = 1;
    imageInfo.extent.height = 1;
    imageInfo.extent.depth = 1;
    imageInfo.format = VK_FORMAT_R32_SFLOAT;
    imageInfo.imageType = VK_IMAGE_TYPE_1D;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.mipLevels = 1;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT;    

    result = vkCreateImage(m_device, &imageInfo, nullptr, &image);

    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create temp image!");
    
    }

    vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);

    allocInfo.memoryTypeIndex = 
        m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, RESOURCE_MEMORY_USAGE_GPU_ONLY);

    allocInfo.allocationSize = R_ALLOC_MASK(desc.texturePoolGPUOnly, memoryRequirements.alignment);

    R_DEBUG(R_CHANNEL_VULKAN, "Allocating device memory...");

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, 
        &m_imagePool[RESOURCE_MEMORY_USAGE_GPU_ONLY].memory);
    
    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate device memory!")
    
    }

    m_imagePool[RESOURCE_MEMORY_USAGE_GPU_ONLY].sizeBytes = allocInfo.allocationSize;

    vkDestroyImage(m_device, image, nullptr);

    // TODO: Need to add allocator for images later.
    m_imageAllocators[RESOURCE_MEMORY_USAGE_GPU_ONLY] = new VulkanAllocator();
    m_imageAllocators[RESOURCE_MEMORY_USAGE_GPU_ONLY]->initialize(new StackAllocator(),
        &m_imagePool[RESOURCE_MEMORY_USAGE_GPU_ONLY]);
#endif
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

    ErrType err = pQueue->initialize(this, queueFamilyIndex, queueIndex);

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

        if (result != REC_RESULT_OK) {

            delete pBuffer;

        } else {

            *ppResource = pBuffer;

        }
    } else {
    
        VulkanImage* pImage = new VulkanImage(desc);

        result = pImage->initialize(this, desc);

        if (result != REC_RESULT_OK) {

            delete pImage;

        } else {

            *ppResource = pImage;

        }
    }
    return result;
}


ErrType VulkanDevice::destroyResource(GraphicsResource* pResource)
{
    if (pResource) {
        VulkanResource* pVr = static_cast<VulkanResource*>(pResource);
        
        pVr->destroy();
        
        delete pResource;
        return REC_RESULT_OK;

    }

    return REC_RESULT_FAILED;
}


ErrType VulkanDevice::createCommandPools(U32 buffers)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Creating command pools...");

    VkCommandPoolCreateInfo poolIf  = { };
    poolIf.sType                    = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolIf.flags                    = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) {

        m_queueFamilies[i].commandPools.resize(buffers);

        poolIf.queueFamilyIndex = m_queueFamilies[i].queueFamilyIndex;

        for (U32 j = 0; j < m_queueFamilies[i].commandPools.size(); ++j) {
    
            VkResult result = vkCreateCommandPool(m_device, &poolIf, nullptr, 
                &m_queueFamilies[i].commandPools[j]);

            if (result != VK_SUCCESS) {

                R_ERR(R_CHANNEL_VULKAN, "Failed to create command pool for queue family...");

            }

        }
    
    }

    return REC_RESULT_OK;
}


void VulkanDevice::destroyCommandPools()
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying command pools...");
    
    for (U32 i = 0; i < m_queueFamilies.size(); ++i) {
 
        for (U32 j = 0; j < m_queueFamilies[i].commandPools.size(); ++j) {

            if (m_queueFamilies[i].commandPools[j]) {

                vkDestroyCommandPool(m_device, m_queueFamilies[i].commandPools[j], nullptr);
                m_queueFamilies[i].commandPools[j] = VK_NULL_HANDLE;

            }
        }

    }
}


ErrType VulkanDevice::createCommandList(GraphicsCommandList** pList, GraphicsQueueTypeFlags flags)
{
    ErrType result = REC_RESULT_OK;

    R_DEBUG(R_CHANNEL_VULKAN, "Creating command list...");

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) {
    
        GraphicsQueueTypeFlags famFlags = m_queueFamilies[i].flags;

        if (flags & famFlags) {
            U32 queueFamilyIndex        = m_queueFamilies[i].queueFamilyIndex;
            VulkanCommandList* pVList   = new VulkanCommandList();

            result = pVList->initialize(this, queueFamilyIndex, 
                m_queueFamilies[i].commandPools.data(), 
                (U32)m_queueFamilies[i].commandPools.size());

            if (result != REC_RESULT_OK) {
            
                R_ERR(R_CHANNEL_VULKAN, "Could not create CommandList...");

                pVList->destroy(this);
                delete pVList;

                return result;
            
            }

            *pList = pVList;
            break;
        }    

    }

    return result;
}


ErrType VulkanDevice::destroyCommandList(GraphicsCommandList* pList)
{
    if (pList) {

        R_DEBUG(R_CHANNEL_VULKAN, "Destroying command list...");

        VulkanCommandList* pVc = static_cast<VulkanCommandList*>(pList);

        pVc->destroy(this);
        delete pVc;

        return REC_RESULT_OK;
    }

    return REC_RESULT_FAILED;
}


void VulkanDevice::prepare()
{
    U32 currentBufferIndex = getCurrentBufferIndex();

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) {
    
        VkCommandPool pool = m_queueFamilies[i].commandPools[currentBufferIndex];
        VkResult result = vkResetCommandPool(m_device, pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        if (result != VK_SUCCESS) {
        
            R_ERR(R_CHANNEL_VULKAN, "Failed to reset command pool!");
        
        }
    
    }
}


void VulkanDevice::createFences(U32 buffering)
{
    m_fences.resize(buffering);
    for (U32 i = 0; i < m_fences.size(); ++i) {
    
        VkFenceCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = 0;
        vkCreateFence(m_device, &info, nullptr, &m_fences[i]);
    
    }
}


void VulkanDevice::destroyFences()
{
    for (U32 i = 0; i < m_fences.size(); ++i) {
    
        vkDestroyFence(m_device, m_fences[i], nullptr);
    
    }
}


ErrType VulkanDevice::createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc)
{
    VulkanResourceView* pView = new VulkanResourceView(desc);
    ErrType err               = pView->initialize(this);

    if (err != REC_RESULT_OK) {
    
        pView->destroy(this);
        delete pView;
    
    }

    *ppView = pView;

    return err;
}


ErrType VulkanDevice::destroyResourceView(GraphicsResourceView* pView)
{
    ErrType result = REC_RESULT_OK;

    if (!pView) {
    
        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    VulkanResourceView* pVv = static_cast<VulkanResourceView*>(pView);
    result                  = pVv->destroy(this);

    if (result != REC_RESULT_OK) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to destroy vulkan image view!");

        return result;

    }

    delete pVv;    

    return result;
}
} // Recluse