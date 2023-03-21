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


void checkAvailableDeviceExtensions(const VulkanAdapter* adapter, std::vector<const char*>& extensions)
{
}


void VulkanContext::initialize(U32 bufferCount)
{    
    VulkanDevice* pDevice = m_pDevice;

    release();

    // Create the command pool.
    createFences(bufferCount);
    createCommandPools(bufferCount);
    createPrimaryCommandList(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
    DescriptorAllocator* descriptorAllocator = pDevice->getDescriptorAllocator();
    // We are essentially reserving descriptor allocator instances.
    descriptorAllocator->initialize(pDevice, bufferCount);
}


ErrType VulkanContext::setBuffers(U32 bufferCount)
{
    initialize(bufferCount);
    m_bufferCount = bufferCount;
    return RecluseResult_Ok;
}


void VulkanContext::release()
{
    destroyPrimaryCommandList();
    destroyCommandPools();
    destroyFences();
}


GraphicsDevice* VulkanContext::getDevice()
{
    return m_pDevice;
}


DescriptorAllocatorInstance* VulkanContext::currentDescriptorAllocator()
{
    return m_pDevice->getDescriptorAllocatorInstance(getCurrentBufferIndex());
}


void VulkanContext::begin()
{    
    R_ASSERT(getBufferCount() > 0);
    VulkanSwapchain* pSwapchain = getNativeDevice()->getSwapchain()->castTo<VulkanSwapchain>();
    const U32 numFrames         = pSwapchain->getFrameCount();
    const U32 bufferCount       = getBufferCount();

    pSwapchain->prepareFrame((numFrames > bufferCount ? getCurrentFence() : VK_NULL_HANDLE));
    prepare();

    m_primaryCommandList.use(getCurrentBufferIndex());
    m_primaryCommandList.reset();
    m_primaryCommandList.begin();
}


void VulkanContext::end()
{
    endRenderPass(m_primaryCommandList.get());
    m_primaryCommandList.end();
    // This performance our submittal.
    VulkanDevice* pDevice       = getNativeDevice();
    VulkanSwapchain* pSwapchain = pDevice->getSwapchain()->castTo<VulkanSwapchain>();
    const U32 frameCount        = pSwapchain->getFrameCount();
    const U32 bufferCount       = getBufferCount();
    VkFence fence               = (frameCount > bufferCount ? getCurrentFence() : VK_NULL_HANDLE);

    // Flush all copies down for this run.
    m_pDevice->flushAllMappedRanges();
    m_pDevice->invalidateAllMappedRanges();

    pSwapchain->submitFinalCommandBuffer(m_primaryCommandList.get(), fence);
    incrementBufferIndex();
}


ErrType VulkanDevice::initialize(VulkanAdapter* adapter, DeviceCreateInfo& info)
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 

    VkDeviceCreateInfo createInfo                       = { };

    VulkanInstance* pVc                                  = adapter->getInstance();
    std::vector<VkQueueFamilyProperties> queueFamilies  = adapter->getQueueFamilyProperties();
    std::vector<const char*> deviceExtensions           = adapter->queryAvailableDeviceExtensions(pVc->getRequestedDeviceFeatures());

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
    createDescriptorHeap();
    allocateMemCache();

    m_allocationManager = makeSmartPtr(new VulkanAllocationManager());
    ErrType err = m_allocationManager->initialize(this);

    if (err != RecluseResult_Ok)
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to initialize the allocation manager!");
    }
    
    {
        VulkanAllocationManager::UpdateConfig config;
        config.flags = VulkanAllocationManager::Flag_GarbageResize;
        config.garbageBufferCount = info.buffering;
        config.frameIndex = 0; 
        m_allocationManager->update(config);
    }

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
    
    DescriptorSets::clearDescriptorLayoutCache(this);
    m_allocationManager->release();

    destroyQueues();
    destroyDescriptorHeap();
    freeMemCache();
    RenderPasses::clearCache(this);
    ShaderPrograms::unloadAll(this);
    Pipelines::VertexLayout::unloadAll();
    Pipelines::clearPipelineCache(this);

    if (m_surface) 
    {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed surface.");
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
    m_allocationManager->setTotalMemory(desc);
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


ErrType VulkanContext::createCommandPools(U32 buffers)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Creating command pools...");

    VkCommandPoolCreateInfo poolIf                  = { };
    poolIf.sType                                    = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolIf.flags                                    = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkQueueFlags queueFlags                         = (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
    const std::vector<QueueFamily>& queueFamilies   = m_pDevice->getQueueFamilies(); 
    VkResult result                                 = VK_SUCCESS;

    for (U32 i = 0; i < queueFamilies.size(); ++i) 
    {
        if (queueFamilies[i].flags & queueFlags)
        {
            poolIf.queueFamilyIndex = queueFamilies[i].queueFamilyIndex;
            m_queueFamilyIndex      = poolIf.queueFamilyIndex;
            m_commandPools.resize(buffers);
            for (U32 j = 0; j < m_commandPools.size(); ++j) 
            { 
                result = vkCreateCommandPool
                                    (
                                        m_pDevice->get(),
                                        &poolIf, 
                                        nullptr, 
                                        &m_commandPools[j]
                                    );

                if (result != VK_SUCCESS) 
                {
                    R_ERR(R_CHANNEL_VULKAN, "Failed to create command pool for queue family...");
                    destroyCommandPools();
                    break;
                }
            }
            
            break;
        }
    }

    return ((result != VK_SUCCESS) ? RecluseResult_Failed : RecluseResult_Ok);
}


void VulkanContext::destroyCommandPools()
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying command pools...");
    VulkanDevice* pDevice = getNativeDevice();
    for (U32 i = 0; i < m_commandPools.size(); ++i) 
    {
        if (m_commandPools[i] != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(pDevice->get(), m_commandPools[i], nullptr);
        }
        m_commandPools[i] = VK_NULL_HANDLE;
    }
    m_commandPools.clear();
}


ErrType VulkanContext::createPrimaryCommandList(VkQueueFlags flags)
{
    ErrType result = RecluseResult_Ok;
    U32 queueFamilyIndex = m_queueFamilyIndex;
    R_DEBUG(R_CHANNEL_VULKAN, "Creating command list...");
    result = m_primaryCommandList.initialize
                (
                    this,
                    queueFamilyIndex, 
                    m_commandPools.data(), 
                    (U32)m_commandPools.size()
                );

    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Could not create CommandList...");

        m_primaryCommandList.release(this);

        return result;
    }
    return result;
}


ErrType VulkanContext::destroyPrimaryCommandList()
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying command list...");
    m_primaryCommandList.release(this);
    return RecluseResult_Ok;
}


void VulkanContext::resetCommandPool(U32 bufferIdx, Bool resetAllResources)
{
    VkCommandPool commandPool = m_commandPools[bufferIdx];
    VkCommandPoolResetFlags flags = 0;
    if (resetAllResources)
    {
        flags |= VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT;
    }
    vkResetCommandPool(m_pDevice->get(), commandPool, flags);
}


void VulkanContext::prepare()
{
    // NOTE(): Get the current buffer index, this is usually the buffer that we recently have 
    // access to.
    U32 currentBufferIndex = getCurrentBufferIndex();

    // Reset the current buffer's command pools.
    resetCommandPool(currentBufferIndex, true);
    DescriptorSets::clearDescriptorSetCache(this);

    const VulkanAllocationManager::Flags allocUpdate = (VulkanAllocationManager::Flag_SetFrameIndex & VulkanAllocationManager::Flag_Update);

    VulkanAllocationManager::UpdateConfig config;

    config.flags                = allocUpdate;
    config.frameIndex           = currentBufferIndex;
    config.garbageBufferCount   = m_bufferCount;

    m_pDevice->getAllocationManager()->update(config);
    
    // TODO: probably want to figure out a cleaner way of doing this.
    DescriptorSets::clearDescriptorSetCache(this, DescriptorSets::ClearCacheFlag_DescriptorPoolFastClear);
    resetBinds();
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
    m_fences.clear();
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
    // TODO: Need to do a resize of this, and probably not rely on context so much on obtaining the buffer count.
    //       Maybe we should move the descriptor allocator to the context?
    m_descriptorAllocator.initialize(this, 1);
}


void VulkanDevice::destroyDescriptorHeap()
{
    m_descriptorAllocator.release(this);
}


void VulkanDevice::allocateMemCache()
{
    // TODO: In the future, we might need to consider multithreading cases, although
    //       I don't think we will have more than one main rendering thread.
    U64 cacheSizeBytes              = align(sizeof(VkMappedMemoryRange) * 128ull, pointerSizeBytes());
    m_memCache.flush.pool           = new MemoryPool(cacheSizeBytes);
    m_memCache.invalid.pool         = new MemoryPool(cacheSizeBytes);
    m_memCache.flush.allocator      = new LinearAllocator();
    m_memCache.invalid.allocator    = new LinearAllocator();
    PtrType alignedFlushAddress     = align(m_memCache.flush.pool->getBaseAddress(), pointerSizeBytes());
    PtrType alignedInvalidAddress   = align(m_memCache.invalid.pool->getBaseAddress(), pointerSizeBytes());

    m_memCache.flush.allocator->initialize(alignedFlushAddress, cacheSizeBytes);
    m_memCache.invalid.allocator->initialize(alignedInvalidAddress, cacheSizeBytes);
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

    // Ensure to obtain the aligned base memory address, since the first entry will need to be so.
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

    // Ensure to obtain the aligned base memory address, since the first entry will need to be so.
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
    // Alignment of 0 since we want to pack these ranges together. We hope that 
    // the api struct itself will be aligned properly.
    ErrType result          = m_memCache.flush.allocator->allocate
                                (
                                    &allocation, 
                                    sizeof(VkMappedMemoryRange), 
                                    0
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
    pRange->size                = align(mappedRange.size, atomSz);
}


void VulkanDevice::pushInvalidateMemoryRange(const VkMappedMemoryRange& mappedRange)
{
    Allocation allocation   = { };
    VkDeviceSize atomSz     = getNonCoherentSize();
    // Alignement of 0 since we want to pack these ranges together. We hope that the api struct itself will be aligned properly.
    ErrType result          = m_memCache.invalid.allocator->allocate
                                (
                                    &allocation, 
                                    sizeof(VkMappedMemoryRange), 
                                    0
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

    pRange->size                = align(mappedRange.size, atomSz);
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


GraphicsContext* VulkanDevice::createContext()
{
    m_allocatedContexts.reserve(kMaxGraphicsContexts);
    if (m_allocatedContexts.size() >= kMaxGraphicsContexts)
    {
        R_ERR(R_CHANNEL_VULKAN, "Reached maximum allowable graphics contexts to create!");
        return nullptr;
    }

    VulkanContext* pContext = new VulkanContext(this);
    m_allocatedContexts.push_back(pContext);
    return pContext;
}


ErrType VulkanDevice::releaseContext(GraphicsContext* pContext)
{
    VulkanContext* pVc = static_cast<VulkanContext*>(pContext);
    pVc->release();
    delete pVc;
    return RecluseResult_Ok;
}


void VulkanDevice::copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, const CopyBufferRegion* regions, U32 numRegions)
{
    m_swapchain->getPresentationQueue()->copyBufferRegions(dst, src, regions, numRegions);
}
} // Recluse