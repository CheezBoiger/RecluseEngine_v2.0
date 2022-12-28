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


void VulkanContext::initialize()
{
    // Create the command pool.
    createFences(m_bufferCount);
    createPrimaryCommandList(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
}


void VulkanContext::release()
{
    destroyFences();
    destroyPrimaryCommandList();
}


GraphicsDevice* VulkanContext::getDevice()
{
    return m_pDevice;
}


void VulkanContext::begin()
{
    incrementBufferIndex();

    VkFence frameFence  = getCurrentFence();
    VkResult result     = vkWaitForFences(m_pDevice->get(), 1, &frameFence, VK_TRUE, UINT64_MAX);

    if (result != RecluseResult_Ok)
    {
        R_WARN(R_CHANNEL_VULKAN, "Fence wait failed...");
    }

    vkResetFences(m_pDevice->get(), 1, &frameFence);

    R_ASSERT(getBufferCount() > 0);

    prepare();

    m_primaryCommandList.use(getCurrentBufferIndex());
    m_primaryCommandList.reset();
    m_primaryCommandList.begin();
}


void VulkanContext::end()
{
    m_primaryCommandList.end();
    resetBinds();
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

        if (builtSurface != RecluseResult_Ok) 
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

    createQueues();
    createCommandPools(info.buffering);
    createDescriptorHeap();
    allocateMemCache();
    
    m_context = new VulkanContext(this, info.buffering);

    // Create a swapchain if we have our info.
    if (info.winHandle) 
    {
        createSwapchain(&m_swapchain, info.swapchainDescription, getBackbufferQueue());
    }

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

    m_context->release();

    if (m_context)
    { 
        delete m_context;
        m_context = nullptr;
    }

    destroyQueues();
    destroyCommandPools();
    destroyDescriptorHeap();
    freeMemCache();
    ShaderPrograms::unloadAll(this);

    if (m_surface) 
    {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed surface.");
    }

    for (U32 i = 0; i < ResourceMemoryUsage_Count; ++i) 
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
        const SwapchainCreateDescription& pDesc,
        VulkanQueue* pPresentationQueue
    )
{

    VulkanSwapchain* pSwapchain     = new VulkanSwapchain(pDesc, pPresentationQueue);
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
    ErrType result = RecluseResult_Ok;

    if (!pSwapchain) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Null pointer exception with either pContext or pSwapchain.");   
        return RecluseResult_NullPtrExcept;
    }

    VulkanQueue* pPq = pSwapchain->getPresentationQueue();

    if (pPq) 
    {

        pPq->wait();

    }

    result = pSwapchain->destroy();

    if (result != RecluseResult_Ok) 
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
        
        return RecluseResult_Ok;    
    }

    if (!handle) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Null window handle for surface creation.");
        return RecluseResult_NullPtrExcept;
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
        return RecluseResult_Failed;
    }
#endif

    return RecluseResult_Ok;    
}


ErrType VulkanDevice::reserveMemory(const MemoryReserveDesc& desc)
{
    VkMemoryRequirements memoryRequirements = { };

    R_DEBUG
        (
            R_CHANNEL_VULKAN, 
            "Reserving memory for:\n\tHost Buffer Memory (Bytes): \t%llu\n"
            "\n\tDevice Buffer Memory (Bytes): \t%llu\n\tDevice Texture Memory (Bytes): \t%llu", 
            desc.bufferPools[ResourceMemoryUsage_CpuOnly], 
            desc.bufferPools[ResourceMemoryUsage_GpuOnly], 
            desc.texturePoolGPUOnly
        );

    R_DEBUG
        (
            R_CHANNEL_VULKAN, 
            "Total available memory (GB):\n\tDevice: %f\n\tHost: %f", 
            F32(desc.bufferPools[ResourceMemoryUsage_GpuOnly] + desc.texturePoolGPUOnly) / R_1GB,
            F32(desc.bufferPools[ResourceMemoryUsage_CpuOnly]) / R_1GB
        );

    VkMemoryAllocateInfo allocInfo = { };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VkBuffer buffer;
    VkImage image;
    VkResult result = VK_SUCCESS;

    for (U32 i = 0; i < ResourceMemoryUsage_Count; ++i) 
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

        if (i != ResourceMemoryUsage_GpuOnly) 
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
                m_context->getBufferCount()
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
        m_adapter->findMemoryType(memoryRequirements.memoryTypeBits, ResourceMemoryUsage_GpuOnly);

    allocInfo.allocationSize = R_ALLOC_MASK(desc.texturePoolGPUOnly, memoryRequirements.alignment);

    R_DEBUG(R_CHANNEL_VULKAN, "Allocating device memory...");

    result = vkAllocateMemory
                (
                    m_device, 
                    &allocInfo, 
                    nullptr, 
                    &m_imagePool[ResourceMemoryUsage_GpuOnly].memory
                );
    
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate device memory!");
    }

    m_imagePool[ResourceMemoryUsage_GpuOnly].sizeBytes = allocInfo.allocationSize;

    vkDestroyImage(m_device, image, nullptr);

    // TODO: Need to add allocator for images later.
    m_imageAllocators[ResourceMemoryUsage_GpuOnly] = new VulkanAllocator();
    m_imageAllocators[ResourceMemoryUsage_GpuOnly]->initialize
        (
            new LinearAllocator(),
            &m_imagePool[ResourceMemoryUsage_GpuOnly], 
            m_context->getBufferCount()
        );
#endif
    return RecluseResult_Ok;
}


ErrType VulkanDevice::createQueues()
{
    // Lets create the primary queue.
    ErrType result          = RecluseResult_Ok;
    VkQueueFlags flags      = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    B32 isPresentSupported  = false;

    if (hasSurface()) 
    {
        isPresentSupported = true;
    }

    result = createQueue(&m_pGraphicsQueue, flags, isPresentSupported);

    if (result != RecluseResult_Ok) 
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

        return RecluseResult_Failed;
    }

    pQueue = new VulkanQueue(flags, isPresentable);

    ErrType err = pQueue->initialize(this, pFamily, queueFamilyIndex, queueIndex);

    if (err == RecluseResult_Ok) 
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

    return RecluseResult_Ok;
}


ErrType VulkanDevice::createResource(GraphicsResource** ppResource, GraphicsResourceDescription& desc, ResourceState initState)
{
    VulkanResource* pResource = Resources::makeResource(this, desc, initState);
    *ppResource = pResource;
    return pResource ? RecluseResult_Ok : RecluseResult_Failed;
}


ErrType VulkanDevice::destroyResource(GraphicsResource* pResource)
{
    if (pResource) 
    {
        return Resources::releaseResource(this, pResource->getId());
    }

    return RecluseResult_Failed;
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
                                    get(),
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

    return RecluseResult_Ok;
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
                vkDestroyCommandPool(get(), m_queueFamilies[i].commandPools[j], nullptr);
                m_queueFamilies[i].commandPools[j] = VK_NULL_HANDLE;
            }
        }
    }
}


ErrType VulkanContext::createPrimaryCommandList(VkQueueFlags flags)
{
    ErrType result = RecluseResult_Ok;

    R_DEBUG(R_CHANNEL_VULKAN, "Creating command list...");

    const std::vector<QueueFamily>& queueFamilies = m_pDevice->getQueueFamilies();

    for (U32 i = 0; i < queueFamilies.size(); ++i) 
    {
        VkQueueFlags famFlags = queueFamilies[i].flags;

        if (flags & famFlags) 
        {
            U32 queueFamilyIndex        = queueFamilies[i].queueFamilyIndex;

            result = m_primaryCommandList.initialize
                        (
                            this,
                            queueFamilyIndex, 
                            queueFamilies[i].commandPools.data(), 
                            (U32)queueFamilies[i].commandPools.size()
                        );

            if (result != RecluseResult_Ok) 
            {
                R_ERR(R_CHANNEL_VULKAN, "Could not create CommandList...");

                m_primaryCommandList.release(this);

                return result;
            }

            break;
        }    

    }

    return result;
}


ErrType VulkanContext::destroyPrimaryCommandList()
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying command list...");
    m_primaryCommandList.release(this);
    return RecluseResult_Ok;
}


void VulkanContext::prepare()
{
    // NOTE(): Get the current buffer index, this is usually the buffer that we recently have 
    // access to.
    U32 currentBufferIndex = getCurrentBufferIndex();

    // Reset the current buffer's command pools.
    const std::vector<QueueFamily>& queueFamilies = m_pDevice->getQueueFamilies();
    for (U32 i = 0; i < queueFamilies.size(); ++i) 
    {
        VkCommandPool pool = queueFamilies[i].commandPools[currentBufferIndex];
        VkResult result = vkResetCommandPool(m_pDevice->get(), pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        if (result != VK_SUCCESS) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to reset command pool!");
        }
    }

    // Reset this current command list.
    m_primaryCommandList.setStatus(CommandList_Reset);

    const VulkanAllocator::VulkanAllocUpdateFlags allocUpdate = 
          VulkanAllocator::VulkanAllocUpdateFlag_SetFrameIndex 
        & VulkanAllocator::VulkanAllocUpdateFlag_Update;

    VulkanAllocator::UpdateConfig config;

    config.flags                = allocUpdate;
    config.frameIndex           = currentBufferIndex;
    config.garbageBufferCount   = m_bufferCount;

    m_pDevice->updateAllocators(config);
}


void VulkanDevice::updateAllocators(const VulkanAllocator::UpdateConfig& config)
{
    // Call up allocators to update.
    for (U32 i = 0; i < ResourceMemoryUsage_Count; ++i) 
    {
        if (m_bufferAllocators[i])
            m_bufferAllocators[i]->update(config);
    }

    for (U32 i = 0; i < ResourceMemoryUsage_Count; ++i) 
    {
        if (m_imageAllocators[i])
            m_imageAllocators[i]->update(config);
    }
}


void VulkanContext::createFences(U32 buffering)
{
    m_fences.resize(buffering);
    for (U32 i = 0; i < m_fences.size(); ++i) 
    {
        // Create fences with signalled bit, in order for the swapchain to properly 
        // wait on our fences, this should handle initial startup of application rendering, and not cause a block.
        VkFenceCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(m_pDevice->get(), &info, nullptr, &m_fences[i]);
    }
}


void VulkanContext::destroyFences()
{
    for (U32 i = 0; i < m_fences.size(); ++i) 
    {
        vkDestroyFence(m_pDevice->get(), m_fences[i], nullptr);
    }
}


ErrType VulkanDevice::createResourceView(GraphicsResourceView** ppView, const ResourceViewDescription& desc)
{
    VulkanResourceView* pView = ResourceViews::makeResourceView(this, desc);
    if (!pView)
       return RecluseResult_Failed;
    *ppView = pView;
    return RecluseResult_Ok;
}


ErrType VulkanDevice::destroyResourceView(GraphicsResourceView* pView)
{
    if (!pView) return RecluseResult_NullPtrExcept;
    return ResourceViews::releaseResourceView(this, pView->getId());
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
 
    if (result != RecluseResult_Ok) 
    {
        if (result == RecluseResult_OutOfMemory) 
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
 
    if (result != RecluseResult_Ok) 
    {
        if (result == RecluseResult_OutOfMemory) 
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


GraphicsSwapchain* VulkanDevice::getSwapchain()
{
    return m_swapchain;
}


ErrType VulkanContext::wait()
{
    m_pDevice->getBackbufferQueue()->wait();
    return RecluseResult_Ok;
}


ErrType VulkanDevice::createSampler(GraphicsSampler** ppSampler, const SamplerCreateDesc& desc)
{
    VulkanSampler* pVSampler    = ResourceViews::makeSampler(this, desc);
    if (!pVSampler) return RecluseResult_Failed;
    *ppSampler = pVSampler;
    return RecluseResult_Ok;
}

ErrType VulkanDevice::destroySampler(GraphicsSampler* pSampler)
{
    if (!pSampler) return RecluseResult_NullPtrExcept;
    return ResourceViews::releaseSampler(this, pSampler->getId());
}


ErrType VulkanDevice::loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition)
{
    if (ShaderPrograms::isProgramCached(program, permutation))
    {
        return RecluseResult_NeedsUpdate;
    }
    return ShaderPrograms::loadNativeShaderProgramPermutation(this, program, permutation, definition);
}


ErrType VulkanDevice::unloadShaderProgram(ShaderProgramId program)
{
    if (!ShaderPrograms::isProgramCached(program))
    {
        return RecluseResult_Ok;
    }
    return ShaderPrograms::unloadProgram(this, program);
}


void VulkanDevice::unloadAllShaderPrograms()
{
    ShaderPrograms::unloadAll(this);
}


Bool VulkanDevice::makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout)
{
    return Pipelines::VertexLayout::make(id, layout);
}


Bool VulkanDevice::destroyVertexLayout(VertexInputLayoutId id)
{
    return false;
}
} // Recluse