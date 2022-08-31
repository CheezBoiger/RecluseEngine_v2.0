//
#include "VulkanDevice.hpp"
#include "VulkanInstance.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanQueue.hpp"
#include "VulkanResource.hpp"
#include "VulkanViews.hpp"
#include "VulkanCommandList.hpp"
#include "VulkanObjects.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanDescriptorManager.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"

#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/BuddyAllocator.hpp"

namespace Recluse {


std::vector<const char*> getDeviceExtensions() 
{
    return { };    
}


void checkAvailableDeviceExtensions(const VulkanAdapter* adapter, std::vector<const char*>& extensions)
{
    std::vector<VkExtensionProperties> deviceExtensions = adapter->getDeviceExtensionProperties();

    // Query all device extensions available for this device.
    for (U32 i = 0; i < extensions.size(); ++i) 
    {
        B32 found = false;
        for (U32 j = 0; j < deviceExtensions.size(); ++j) 
        { 
            if (strcmp(deviceExtensions[j].extensionName, extensions[i]) == 0) 
            {
                R_DEBUG
                    (
                        R_CHANNEL_VULKAN, 
                        "Found %s Spec Version: %d", 
                        deviceExtensions[j].extensionName,
                        deviceExtensions[j].specVersion
                    );

                found = true;
                break;
            }
    
        }

        if (!found) 
        {
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

    VulkanInstance* pVc                                  = adapter->getInstance();
    std::vector<VkQueueFamilyProperties> queueFamilies  = adapter->getQueueFamilyProperties();
    std::vector<const char*> deviceExtensions           = getDeviceExtensions();

    createInfo.sType                                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    if (info.winHandle) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Creating surface handle.");

        // Create surface
        ErrType builtSurface = createSurface(pVc->get(), info.winHandle);

        if (builtSurface != R_RESULT_OK) 
        {
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

    for (U32 i = 0; i < queueFamilies.size(); ++i) 
    {
        QueueFamily             queueFamily     = { };
        VkDeviceQueueCreateInfo queueInfo       = { };
        VkQueueFamilyProperties queueFamProps   = queueFamilies[i];
        B32 shouldCreateQueues                  = false;

        queueFamily.isPresentSupported  = false;
        queueFamily.flags               = queueFamProps.queueFlags;
        queueInfo.queueCount            = 0;
        queueInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        if (m_surface) 
        {
            if (adapter->checkSurfaceSupport(i, m_surface)) 
            {   
                R_DEBUG(R_CHANNEL_VULKAN, "Device supports present...");

                queueInfo.queueCount += 1;
                queueFamily.isPresentSupported = true;
                shouldCreateQueues = true;
            }
        
        }

        if (queueFamProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            queueInfo.queueCount += 1;
            shouldCreateQueues = true;
        } 
    
        if (queueFamProps.queueFlags & VK_QUEUE_COMPUTE_BIT) 
        {
            queueInfo.queueCount += 1;
            shouldCreateQueues = true;
        } 

        if (queueFamProps.queueFlags & VK_QUEUE_TRANSFER_BIT) 
        {
            queueInfo.queueCount += 1;
            shouldCreateQueues = true;
        }

        if (shouldCreateQueues) 
        {
            // Check if the queue count is too big!
            queueInfo.queueCount = ((queueInfo.queueCount > queueFamilies[i].queueCount) 
                ? queueFamilies[i].queueCount 
                : queueInfo.queueCount);
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

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan device!");   
        return -1;
    }

    m_adapter = adapter;

    // Create the command pool.
    createCommandPools(info.buffering);    
    createFences(info.buffering);
    createDescriptorHeap();
    allocateMemCache();
    createQueues();
    
    // Create a swapchain if we have our info.
    if (info.winHandle) 
    {
        createSwapchain(&m_swapchain, info.swapchainDescription);
    }

    m_bufferCount = info.buffering;

    createCommandList(&m_pPrimaryCommandList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);

    m_properties = adapter->getProperties();

    return 0;
}


void VulkanDevice::release(VkInstance instance)
{
    vkDeviceWaitIdle(m_device);

    if (m_swapchain) 
    {
        destroySwapchain(m_swapchain);
        m_swapchain = nullptr;
    }

    destroyQueues();
    destroyCommandPools();
    destroyFences();
    destroyDescriptorHeap();
    freeMemCache();

    m_cache.clearCache(this);

    if (m_surface) 
    {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed surface.");
    }

    for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) 
    {
        if (m_bufferAllocators[i]) 
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing buffer allocator...");

            m_bufferAllocators[i]->destroy();
            delete m_bufferAllocators[i];
            m_bufferAllocators[i] = nullptr;
        }

        if (m_imageAllocators[i]) 
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing image allocator...");

            m_imageAllocators[i]->destroy();
            delete m_imageAllocators[i];
            m_imageAllocators[i] = nullptr;
        }
    
        if (m_bufferPool[i].memory) 
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated pool...");
            
            vkFreeMemory(m_device, m_bufferPool[i].memory, nullptr);
            m_bufferPool[i].memory = nullptr;
        }

        if (m_imagePool[i].memory) 
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Freeing allocated image pool...");

            vkFreeMemory(m_device, m_imagePool[i].memory, nullptr);
            m_imagePool[i].memory = nullptr;
        }
    }

    

    if (m_device != VK_NULL_HANDLE) 
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Device Destroyed.");
    }
}


ErrType VulkanDevice::createSwapchain
    (
        VulkanSwapchain** ppSwapchain,
        const SwapchainCreateDescription& pDesc
    )
{

    VulkanSwapchain* pSwapchain     = new VulkanSwapchain(pDesc, m_pGraphicsQueue);
    VulkanInstance*   pNativeContext = m_adapter->getInstance();

    ErrType result = pSwapchain->build(this);

    if (result != 0) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Swapchain failed to create");
        return -1;
    }

    *ppSwapchain = pSwapchain;

    return result;
}


ErrType VulkanDevice::destroySwapchain(VulkanSwapchain* pSwapchain)
{
    ErrType result = R_RESULT_OK;

    if (!pSwapchain) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Null pointer exception with either pContext or pSwapchain.");   
        return R_RESULT_NULL_PTR_EXCEPT;
    }

    VulkanQueue* pPq = pSwapchain->getPresentationQueue();

    if (pPq) 
    {

        pPq->wait();

    }

    result = pSwapchain->destroy();

    if (result != R_RESULT_OK) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to destroy vulkan swapchain!");
    } 
    else 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed vulkan swapchain...");
        
        delete pSwapchain;
    }


    return result;
}


ErrType VulkanDevice::createSurface(VkInstance instance, void* handle)
{
    VkResult result             = VK_SUCCESS;

    if (m_surface && (handle == m_windowHandle)) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Surface is already created for handle. Skipping surface create...");
        
        return R_RESULT_OK;    
    }

    if (!handle) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Null window handle for surface creation.");
        return R_RESULT_NULL_PTR_EXCEPT;
    }

    if (m_surface) 
    {
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
    
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create win32 surface.");
        return R_RESULT_FAILED;
    }
#endif

    return R_RESULT_OK;    
}


ErrType VulkanDevice::reserveMemory(const MemoryReserveDesc& desc)
{
    VkMemoryRequirements memoryRequirements = { };

    R_DEBUG
        (
            R_CHANNEL_VULKAN, 
            "Reserving memory for:\n\tHost Buffer Memory (Bytes): \t%llu\n"
            "\n\tDevice Buffer Memory (Bytes): \t%llu\n\tDevice Texture Memory (Bytes): \t%llu", 
            desc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY], 
            desc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY], 
            desc.texturePoolGPUOnly
        );

    R_DEBUG
        (
            R_CHANNEL_VULKAN, 
            "Total available memory (GB):\n\tDevice: %f\n\tHost: %f", 
            F32(desc.bufferPools[RESOURCE_MEMORY_USAGE_GPU_ONLY] + desc.texturePoolGPUOnly) / R_1GB,
            F32(desc.bufferPools[RESOURCE_MEMORY_USAGE_CPU_ONLY]) / R_1GB
        );

    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VkBuffer buffer;
    VkImage image;
    VkResult result = VK_SUCCESS;

    for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) 
    {
        // start with memory gpu.
        VkBufferCreateInfo bufferInfo = { };
        bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size         = desc.bufferPools[i];
        bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

        bufferInfo.usage        = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
                                    | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT 
                                    | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
                                    | VK_BUFFER_USAGE_INDEX_BUFFER_BIT 
                                    | VK_BUFFER_USAGE_TRANSFER_DST_BIT 
                                    | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);

        if (result != VK_SUCCESS) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to create temp buffer!");
        }

        vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);

        allocInfo.memoryTypeIndex = 
            m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, (ResourceMemoryUsage)i);

        allocInfo.allocationSize = R_ALLOC_MASK(memoryRequirements.size, memoryRequirements.alignment);

        R_DEBUG(R_CHANNEL_VULKAN, "Allocating device memory...");

        result = vkAllocateMemory
                    (
                        m_device, 
                        &allocInfo, 
                        nullptr, 
                        &m_bufferPool[i].memory
                    );
    
        if (result != VK_SUCCESS) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to allocate device memory!");
        }

        m_bufferPool[i].sizeBytes = allocInfo.allocationSize;

        if (i != RESOURCE_MEMORY_USAGE_GPU_ONLY) 
        {
            vkMapMemory
                (
                    m_device, 
                    m_bufferPool[i].memory, 
                    0, 
                    allocInfo.allocationSize, 
                    0, 
                    &m_bufferPool[i].basePtr
                );
        }

        vkDestroyBuffer(m_device, buffer, nullptr);

        // TODO: Need to add allocator for buffers later.
        m_bufferAllocators[i] = new VulkanAllocator();
        m_bufferAllocators[i]->initialize
            (
                new BuddyAllocator(), 
                &m_bufferPool[i],
                m_bufferCount
            );
    }
    
    // Create image pools.
#if 1
    // start with memory gpu.
    VkImageCreateInfo imageInfo = { };
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.arrayLayers   = 1;
    imageInfo.extent.width  = 1;
    imageInfo.extent.height = 1;
    imageInfo.extent.depth  = 1;
    imageInfo.format        = VK_FORMAT_R32_SFLOAT;
    imageInfo.imageType     = VK_IMAGE_TYPE_1D;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.mipLevels     = 1;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

    imageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
                                | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
                                | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT 
                                | VK_IMAGE_USAGE_SAMPLED_BIT 
                                | VK_IMAGE_USAGE_TRANSFER_DST_BIT 
                                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;    

    result = vkCreateImage(m_device, &imageInfo, nullptr, &image);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create temp image!");
    }

    vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);

    allocInfo.memoryTypeIndex = 
        m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, RESOURCE_MEMORY_USAGE_GPU_ONLY);

    allocInfo.allocationSize = R_ALLOC_MASK(desc.texturePoolGPUOnly, memoryRequirements.alignment);

    R_DEBUG(R_CHANNEL_VULKAN, "Allocating device memory...");

    result = vkAllocateMemory
                (
                    m_device, 
                    &allocInfo, 
                    nullptr, 
                    &m_imagePool[RESOURCE_MEMORY_USAGE_GPU_ONLY].memory
                );
    
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate device memory!");
    }

    m_imagePool[RESOURCE_MEMORY_USAGE_GPU_ONLY].sizeBytes = allocInfo.allocationSize;

    vkDestroyImage(m_device, image, nullptr);

    // TODO: Need to add allocator for images later.
    m_imageAllocators[RESOURCE_MEMORY_USAGE_GPU_ONLY] = new VulkanAllocator();
    m_imageAllocators[RESOURCE_MEMORY_USAGE_GPU_ONLY]->initialize
        (
            new LinearAllocator(),
            &m_imagePool[RESOURCE_MEMORY_USAGE_GPU_ONLY], 
            m_bufferCount
        );
#endif
    return R_RESULT_OK;
}


ErrType VulkanDevice::createQueues()
{
    // Lets create the primary queue.
    ErrType result          = R_RESULT_OK;
    VkQueueFlags flags      = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    B32 isPresentSupported  = false;

    if (m_windowHandle) 
    {
        isPresentSupported = true;
    }

    result = createQueue(&m_pGraphicsQueue, flags, isPresentSupported);

    if (result != R_RESULT_OK) 
    { 
        R_ERR(R_CHANNEL_VULKAN, "Failed to create main RHI queue!");
    }

    return result;
}


ErrType VulkanDevice::createQueue(VulkanQueue** ppQueue, VkQueueFlags flags, B32 isPresentable)
{
    U32 queueFamilyIndex    = 0xFFFFFFFF;
    U32 queueIndex          = 0xFFFFFFFF;
    VulkanQueue* pQueue     = nullptr;
    QueueFamily* pFamily    = nullptr;

    // Main queue first.

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) 
    {
        QueueFamily& family = m_queueFamilies[i];

        if (family.flags & flags) 
        {
            if (isPresentable && !family.isPresentSupported) 
            {
                // If this queue family is not present supported, then check the next.
                continue;
            }

            // Check if we can get a queue from this family.
            if (family.currentAvailableQueueIndex < family.maxQueueCount) 
            {    
                queueFamilyIndex    = family.queueFamilyIndex;
                queueIndex          = family.currentAvailableQueueIndex++;
                pFamily             = &family;
                break;   
            }
        }
    }

    if (queueFamilyIndex == 0xFFFFFFFF || queueIndex == 0xFFFFFFFF) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Could not find proper queue family! Can not create queue.");

        return R_RESULT_FAILED;
    }

    pQueue = new VulkanQueue(flags, isPresentable);

    ErrType err = pQueue->initialize(this, pFamily, queueFamilyIndex, queueIndex);

    if (err == R_RESULT_OK) 
    {
        *ppQueue = pQueue;
    }

    return err;
}


ErrType VulkanDevice::destroyQueues()
{
    if (m_pGraphicsQueue) 
    {
        delete m_pGraphicsQueue;
        m_pGraphicsQueue = nullptr;
    }

    return R_RESULT_OK;
}


ErrType VulkanDevice::createResource(GraphicsResource** ppResource, GraphicsResourceDescription& desc, ResourceState initState)
{
    ErrType result = R_RESULT_OK;

    if (desc.dimension == RESOURCE_DIMENSION_BUFFER) 
    {
        VulkanBuffer* pBuffer = new VulkanBuffer(desc);

        result = pBuffer->initialize(this, desc, initState);

        if (result != R_RESULT_OK) 
        {
            delete pBuffer;
        } 
        else 
        {
            *ppResource = pBuffer;
        }
    } 
    else 
    {
    
        VulkanImage* pImage = new VulkanImage(desc);

        result = pImage->initialize(this, desc, initState);

        if (result != R_RESULT_OK) 
        {
            delete pImage;
        } 
        else 
        {
            *ppResource = pImage;
        }
    }
    return result;
}


ErrType VulkanDevice::destroyResource(GraphicsResource* pResource)
{
    if (pResource) 
    {
        VulkanResource* pVr = static_cast<VulkanResource*>(pResource);
        
        pVr->destroy();
        
        delete pResource;
        return R_RESULT_OK;
    }

    return R_RESULT_FAILED;
}


ErrType VulkanDevice::createCommandPools(U32 buffers)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Creating command pools...");

    VkCommandPoolCreateInfo poolIf  = { };
    poolIf.sType                    = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolIf.flags                    = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) 
    {
        m_queueFamilies[i].commandPools.resize(buffers);

        poolIf.queueFamilyIndex = m_queueFamilies[i].queueFamilyIndex;

        for (U32 j = 0; j < m_queueFamilies[i].commandPools.size(); ++j) 
        { 
            VkResult result = vkCreateCommandPool
                                (
                                    m_device, 
                                    &poolIf, 
                                    nullptr, 
                                    &m_queueFamilies[i].commandPools[j]
                                );

            if (result != VK_SUCCESS) 
            {
                R_ERR(R_CHANNEL_VULKAN, "Failed to create command pool for queue family...");
            }
        }
    }

    return R_RESULT_OK;
}


void VulkanDevice::destroyCommandPools()
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying command pools...");
    
    for (U32 i = 0; i < m_queueFamilies.size(); ++i) 
    {
        for (U32 j = 0; j < m_queueFamilies[i].commandPools.size(); ++j) 
        {
            if (m_queueFamilies[i].commandPools[j]) 
            {
                vkDestroyCommandPool(m_device, m_queueFamilies[i].commandPools[j], nullptr);
                m_queueFamilies[i].commandPools[j] = VK_NULL_HANDLE;
            }
        }
    }
}


ErrType VulkanDevice::createCommandList(VulkanCommandList** pList, VkQueueFlags flags)
{
    ErrType result = R_RESULT_OK;

    R_DEBUG(R_CHANNEL_VULKAN, "Creating command list...");

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) 
    {
        VkQueueFlags famFlags = m_queueFamilies[i].flags;

        if (flags & famFlags) 
        {
            U32 queueFamilyIndex        = m_queueFamilies[i].queueFamilyIndex;
            VulkanCommandList* pVList   = new VulkanCommandList();

            result = pVList->initialize
                        (
                            this, 
                            queueFamilyIndex, 
                            m_queueFamilies[i].commandPools.data(), 
                            (U32)m_queueFamilies[i].commandPools.size()
                        );

            if (result != R_RESULT_OK) 
            {
                R_ERR(R_CHANNEL_VULKAN, "Could not create CommandList...");

                pVList->release(this);
                delete pVList;

                return result;
            }

            *pList = pVList;
            break;
        }    

    }

    return result;
}


ErrType VulkanDevice::destroyCommandList(VulkanCommandList* pList)
{
    if (pList) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying command list...");

        pList->release(this);
        delete pList;

        return R_RESULT_OK;
    }

    return R_RESULT_FAILED;
}


void VulkanDevice::prepare()
{
    // NOTE(): Get the current buffer index, this is usually the buffer that we recently have 
    // access to.
    U32 currentBufferIndex = getCurrentBufferIndex();

    // Reset the current buffer's command pools.
    for (U32 i = 0; i < m_queueFamilies.size(); ++i) 
    {
        VkCommandPool pool = m_queueFamilies[i].commandPools[currentBufferIndex];
        VkResult result = vkResetCommandPool(m_device, pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        if (result != VK_SUCCESS) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to reset command pool!");
        }
    }

    // Reset this current command list.
    m_pPrimaryCommandList->setStatus(COMMAND_LIST_RESET);

    const VulkanAllocator::VulkanAllocUpdateFlags allocUpdate = 
          VulkanAllocator::VULKAN_ALLOC_SET_FRAME_INDEX 
        & VulkanAllocator::VULKAN_ALLOC_UPDATE_FLAG;

    VulkanAllocator::UpdateConfig config;

    config.flags                = allocUpdate;
    config.frameIndex           = currentBufferIndex;
    config.garbageBufferCount   = m_bufferCount;

    // Call up allocators to update.
    for (U32 i = 0; i < RESOURCE_MEMORY_USAGE_COUNT; ++i) 
    {
        if (m_bufferAllocators[i])
            m_bufferAllocators[i]->update(config);
    }

    for (U32 i = 0;i < RESOURCE_MEMORY_USAGE_COUNT; ++i) 
    {
        if (m_imageAllocators[i])
            m_imageAllocators[i]->update(config);
    }
}


void VulkanDevice::createFences(U32 buffering)
{
    m_fences.resize(buffering);
    for (U32 i = 0; i < m_fences.size(); ++i) 
    {
        // Create fences with signalled bit, in order for the swapchain to properly 
        // wait on our fences, this should handle initial startup of application rendering, and not cause a block.
        VkFenceCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(m_device, &info, nullptr, &m_fences[i]);
    }
}


void VulkanDevice::destroyFences()
{
    for (U32 i = 0; i < m_fences.size(); ++i) 
    {
        vkDestroyFence(m_device, m_fences[i], nullptr);
    }
}


ErrType VulkanDevice::createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc)
{
    VulkanResourceView* pView = new VulkanResourceView(desc);
    ErrType err               = pView->initialize(this);

    if (err != R_RESULT_OK) 
    {
        pView->destroy(this);
        delete pView;
    }

    *ppView = pView;

    return err;
}


ErrType VulkanDevice::destroyResourceView(GraphicsResourceView* pView)
{
    ErrType result = R_RESULT_OK;

    if (!pView) 
    {
        return R_RESULT_NULL_PTR_EXCEPT;
    }

    VulkanResourceView* pVv = static_cast<VulkanResourceView*>(pView);
    result                  = pVv->destroy(this);

    if (result != R_RESULT_OK) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to destroy vulkan image view!");

        return result;
    }

    delete pVv;    

    return result;
}


void VulkanDevice::createDescriptorHeap()
{
    if (!m_pDescriptorManager) 
    {
        m_pDescriptorManager = new VulkanDescriptorManager();
        m_pDescriptorManager->initialize(this);
    }
}


void VulkanDevice::destroyDescriptorHeap()
{
    if (m_pDescriptorManager) 
    {
        m_pDescriptorManager->destroy(this);
        delete m_pDescriptorManager;
        m_pDescriptorManager = nullptr;
    }
}


ErrType VulkanDevice::createDescriptorSet(DescriptorSet** ppDescriptorSet, DescriptorSetLayout* pLayout)
{
    VulkanDescriptorSetLayout* pVl      = static_cast<VulkanDescriptorSetLayout*>(pLayout);
    VulkanDescriptorSet* pDescriptorSet = new VulkanDescriptorSet();
    ErrType result                      = pDescriptorSet->initialize(this, pVl);

    if (result != R_RESULT_OK) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Device failed to create vulkan descriptor!");
        delete pVl;
        return result;
    }

    *ppDescriptorSet = pDescriptorSet;

    return result;
}


ErrType VulkanDevice::destroyDescriptorSet(DescriptorSet* pDescriptorSet)
{
    VulkanDescriptorSet* pSet = static_cast<VulkanDescriptorSet*>(pDescriptorSet);
    pSet->destroy();
    delete pSet;
    return R_RESULT_OK;
}


ErrType VulkanDevice::createDescriptorSetLayout(DescriptorSetLayout** ppLayout, const DescriptorSetLayoutDesc& desc)
{
    VulkanDescriptorSetLayout* pVl  = new VulkanDescriptorSetLayout();
    ErrType result                  = pVl->initialize(this, desc);

    if (result != R_RESULT_OK) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan descriptor set layout!");
        pVl->destroy(this);
        delete pVl;
        return result;
    }

    *ppLayout = pVl;

    return R_RESULT_OK;
}


ErrType VulkanDevice::destroyDescriptorSetLayout(DescriptorSetLayout* pLayout)
{
    if (!pLayout) 
    {
        return R_RESULT_NULL_PTR_EXCEPT;
    }

    VulkanDescriptorSetLayout* pVl = static_cast<VulkanDescriptorSetLayout*>(pLayout);
    ErrType result = pVl->destroy(this);

    return result;
}


ErrType VulkanDevice::createRenderPass(RenderPass** ppRenderPass, const RenderPassDesc& desc)
{
    VulkanRenderPass* pVrp = new VulkanRenderPass();
    ErrType result = pVrp->initialize(this, desc);

    if (result != R_RESULT_OK) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan render pass...");
    
        pVrp->destroy(this);
        delete pVrp;

        return result;
    }

    *ppRenderPass = pVrp;

    return result;
}


ErrType VulkanDevice::destroyRenderPass(RenderPass* pRenderPass)
{
    if (!pRenderPass) 
    {
        return R_RESULT_NULL_PTR_EXCEPT;
    }

    VulkanRenderPass* pVrp = static_cast<VulkanRenderPass*>(pRenderPass);
    ErrType result = pVrp->destroy(this);
    delete pVrp;

    if (result != R_RESULT_OK) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to destroy render pass!");
    }

    return result;
}


ErrType VulkanDevice::createGraphicsPipelineState(PipelineState** ppPipelineState, const GraphicsPipelineStateDesc& desc)
{
    VulkanGraphicsPipelineState* pPipeline  = new VulkanGraphicsPipelineState();
    ErrType result                          = R_RESULT_OK;

    result = pPipeline->initialize(this, desc);
    
    if (result != R_RESULT_OK) 
    {
        pPipeline->destroy(this);
        delete pPipeline;

        return result;
    }

    *ppPipelineState = pPipeline;

    return result;
}


ErrType VulkanDevice::destroyPipelineState(PipelineState* pPipelineState)
{
    R_ASSERT(pPipelineState != NULL);

    ErrType result                  = R_RESULT_OK;
    VulkanPipelineState* pPipeline  = static_cast<VulkanPipelineState*>(pPipelineState);
    
    pPipeline->destroy(this);

    delete pPipeline;

    return result;
}


void VulkanDevice::allocateMemCache()
{
    // TODO: In the future, we might need to consider multithreading cases, although
    //       I don't think we will have more than one main rendering thread.
    U64 cacheSizeBytes              = R_ALLOC_MASK(sizeof(VkMappedMemoryRange) * 128ull, ARCH_PTR_SZ_BYTES);
    m_memCache.flush.pool           = new MemoryPool(cacheSizeBytes);
    m_memCache.invalid.pool         = new MemoryPool(cacheSizeBytes);
    m_memCache.flush.allocator      = new LinearAllocator();
    m_memCache.invalid.allocator    = new LinearAllocator();

    m_memCache.flush.allocator->initialize(m_memCache.flush.pool->getBaseAddress(), cacheSizeBytes);
    m_memCache.invalid.allocator->initialize(m_memCache.invalid.pool->getBaseAddress(), cacheSizeBytes);
}


void VulkanDevice::freeMemCache()
{
    if (m_memCache.flush.allocator) 
    {
        m_memCache.flush.allocator->cleanUp();
        delete m_memCache.flush.allocator;
        m_memCache.flush.allocator = nullptr;        
    }
    
    if (m_memCache.flush.pool) 
    {
        delete m_memCache.flush.pool; 
        m_memCache.flush.pool = nullptr;   
    }

    if (m_memCache.invalid.allocator) 
    {
        m_memCache.invalid.allocator->cleanUp();
        delete m_memCache.invalid.allocator;
        m_memCache.invalid.allocator = nullptr;   
    }

    if (m_memCache.invalid.pool) 
    {
        delete m_memCache.invalid.pool;
        m_memCache.invalid.pool = nullptr;
    }
}


void VulkanDevice::flushAllMappedRanges()
{
    if (m_memCache.flush.allocator->getTotalAllocations() == 0) 
    {    
        return;
    }

    VkResult result                 = VK_SUCCESS;
    U32 totalCount                  = (U32)m_memCache.flush.allocator->getTotalAllocations();
    VkMappedMemoryRange* pRanges    = (VkMappedMemoryRange*)m_memCache.flush.allocator->getBaseAddr();

    result = vkFlushMappedMemoryRanges(m_device, totalCount, pRanges);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to flush memory ranges...");    
    }

    m_memCache.flush.allocator->reset();
}


void VulkanDevice::invalidateAllMappedRanges()
{
    if (m_memCache.invalid.allocator->getTotalAllocations() == 0) 
    {    
        return;
    }

    VkResult result                 = VK_SUCCESS;
    U32 totalCount                  = (U32)m_memCache.invalid.allocator->getTotalAllocations();
    VkMappedMemoryRange* pRanges    = (VkMappedMemoryRange*)m_memCache.invalid.allocator->getBaseAddr();

    result = vkInvalidateMappedMemoryRanges(m_device, totalCount, pRanges);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to invalidate memory ranges...");    
    }

    m_memCache.invalid.allocator->reset();
}


void VulkanDevice::pushFlushMemoryRange(const VkMappedMemoryRange& mappedRange)
{
    Allocation allocation   = { };
    VkDeviceSize atomSz     = getNonCoherentSize();
    ErrType result          = m_memCache.flush.allocator->allocate
                                (
                                    &allocation, 
                                    sizeof(VkMappedMemoryRange), 
                                    ARCH_PTR_SZ_BYTES
                                );
 
    if (result != R_RESULT_OK) 
    {
        if (result == R_RESULT_OUT_OF_MEMORY) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Memory flush cache out of memory!");        
        } 
        else 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to push flush memory range!");        
        }
    
        // Possible null ptr returned, breaking off.
        return;
    }

    VkMappedMemoryRange* pRange = (VkMappedMemoryRange*)allocation.baseAddress;
    *pRange                     = mappedRange;

    // We need to align on nonCoherentAtomSize, as spec states it must be a multiple of this.
    pRange->size                = R_ALLOC_MASK(mappedRange.size, atomSz);
}


void VulkanDevice::pushInvalidateMemoryRange(const VkMappedMemoryRange& mappedRange)
{
    Allocation allocation   = { };
    VkDeviceSize atomSz     = getNonCoherentSize();
    ErrType result          = m_memCache.invalid.allocator->allocate
                                (
                                    &allocation, 
                                    sizeof(VkMappedMemoryRange), 
                                    ARCH_PTR_SZ_BYTES
                                );
 
    if (result != R_RESULT_OK) 
    {
        if (result == R_RESULT_OUT_OF_MEMORY) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Memory flush cache out of memory!");        

        } 
        else 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to push flush memory range!");        
        }

        // Possible nullptr, break off.
        return;
    
    }

    VkMappedMemoryRange* pRange = (VkMappedMemoryRange*)allocation.baseAddress;
    *pRange                     = mappedRange;

    pRange->size                = R_ALLOC_MASK(mappedRange.size, atomSz);
}


ErrType VulkanDevice::createComputePipelineState(PipelineState** ppPipelineState, const ComputePipelineStateDesc& desc)
{
    VulkanComputePipelineState* pPipeline = new VulkanComputePipelineState();
    ErrType result = R_RESULT_OK;
    
    result = pPipeline->initialize(this, desc);

    if (result != R_RESULT_OK) 
    {
        pPipeline->destroy(this);
        delete pPipeline;

        return result;
    }

    *ppPipelineState = pPipeline;

    return result;
}


GraphicsCommandList* VulkanDevice::getCommandList()
{
    return m_pPrimaryCommandList;
}


GraphicsSwapchain* VulkanDevice::getSwapchain()
{
    return m_swapchain;
}


ErrType VulkanDevice::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    return m_pGraphicsQueue->copyResource(dst, src);
}

ErrType VulkanDevice::copyBufferRegions
    (
        GraphicsResource* dst, 
        GraphicsResource* src, 
        CopyBufferRegion* pRegions, 
        U32 numRegions
    )
{
    return m_pGraphicsQueue->copyBufferRegions(dst, src, pRegions, numRegions);
}


ErrType VulkanDevice::wait()
{
    m_pGraphicsQueue->wait();
    return R_RESULT_OK;
}


ErrType VulkanDevice::createSampler(GraphicsSampler** ppSampler, const SamplerCreateDesc& desc)
{
    ErrType result              = R_RESULT_OK;
    VulkanSampler* pVSampler    = new VulkanSampler();
    
    result = pVSampler->initialize(this, desc);

    if (result != R_RESULT_OK) 
    {
        pVSampler->destroy(this);
        return result;
    }

    *ppSampler = pVSampler;

    return R_RESULT_OK;
}

ErrType VulkanDevice::destroySampler(GraphicsSampler* pSampler)
{
    if (!pSampler) return R_RESULT_NULL_PTR_EXCEPT;

    ErrType result              = R_RESULT_OK;
    VulkanSampler* pVSampler    = static_cast<VulkanSampler*>(pSampler);

    result = pVSampler->destroy(this);

    delete pVSampler;

    return result;
}
} // Recluse