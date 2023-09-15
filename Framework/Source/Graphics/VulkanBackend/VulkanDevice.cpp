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


R_DECLARE_GLOBAL_BOOLEAN(g_justLog, false, "Vulkan.Test");


void checkAvailableDeviceExtensions(const VulkanAdapter* adapter, std::vector<const char*>& extensions)
{
}


VulkanContext::~VulkanContext()
{
    R_ASSERT_FORMAT((m_bufferCount == 0), "Vulkan context was not properly released! Memory handle still active until end of application life!");
}


void VulkanContext::initialize(U32 bufferCount)
{    
    VulkanDevice* pDevice = m_pDevice;

    release();

    // Create the command pool.
    createContextFrames(bufferCount);
    createCommandPools(bufferCount);
    createPrimaryCommandList(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
    DescriptorAllocator* descriptorAllocator = pDevice->getDescriptorAllocator();
    VulkanAllocationManager* allocManager = pDevice->getAllocationManager();
    // We are essentially reserving descriptor allocator instances.
    descriptorAllocator->initialize(pDevice, bufferCount);

    VulkanAllocationManager::UpdateConfig config = { };
    config.flags = VulkanAllocationManager::Flag_GarbageResize | VulkanAllocationManager::Flag_SetFrameIndex;
    config.garbageBufferCount = bufferCount;
    config.frameIndex = m_currentBufferIndex;
    allocManager->update(config);
}


ResultCode VulkanContext::setFrames(U32 bufferCount)
{
    initialize(bufferCount);
    m_bufferCount = bufferCount;
    return RecluseResult_Ok;
}


void VulkanContext::release()
{
    destroyPrimaryCommandList();
    destroyCommandPools();
    destroyContextFrames();
    // Ensure we no longer have any buffers.
    m_bufferCount = 0;
    m_currentBufferIndex = 0;
}


GraphicsDevice* VulkanContext::getDevice()
{
    return m_pDevice;
}


DescriptorAllocatorInstance* VulkanContext::currentDescriptorAllocator()
{
    return m_pDevice->getDescriptorAllocatorInstance(getCurrentFrameIndex());
}


void VulkanContext::begin()
{    
    R_ASSERT(getFrameCount() > 0);
    incrementBufferIndex();
    VulkanContextFrame& contextFrame    = getContextFrame(getCurrentFrameIndex());
    VkFence frameFence                  = contextFrame.fence;

    // We need to wait for our fences, before we can begin to reset resources.
    vkWaitForFences(m_pDevice->get(), 1, &frameFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_pDevice->get(), 1, &frameFence);

    prepare();

    m_primaryCommandList.use(getCurrentFrameIndex());
    m_primaryCommandList.reset();
    m_primaryCommandList.begin();

    if (g_justLog)
    {
        R_VERBOSE(R_CHANNEL_VULKAN, "We are Logging!!!");
    }
}


void VulkanContext::endRenderPass(VkCommandBuffer buffer)
{
    if (m_boundRenderPass) 
    {
        vkCmdEndRenderPass(buffer);
        m_boundRenderPass = nullptr;
    }   
}


Bool VulkanContext::supportsAsyncCompute() const
{
    return false;
}


void VulkanContext::end()
{
    endRenderPass(m_primaryCommandList.get());
    flushBarrierTransitions(m_primaryCommandList.get());
    m_primaryCommandList.end();

    // Flush all copies down for this run.
    m_pDevice->flushAllMappedRanges();
    m_pDevice->invalidateAllMappedRanges();

    submitFinalCommandBuffer(m_primaryCommandList.get());
}




ResultCode VulkanContext::submitFinalCommandBuffer(VkCommandBuffer commandBuffer)
{
    U32 currentFrameIndex               = getCurrentFrameIndex();
    VkDevice device                     = m_pDevice->get();
    VulkanContextFrame& contextFrame    = getContextFrame(currentFrameIndex);
    VkImageMemoryBarrier imgBarrier     = { };
    VkImageSubresourceRange range       = { };
    VkCommandBuffer primaryCmdBuf       = commandBuffer;
    VkFence fence                       = contextFrame.fence;
    VkSemaphore signalSemaphore         = contextFrame.signalSemaphore;
    VkSemaphore waitSemaphore           = contextFrame.waitSemaphore;

    R_ASSERT(primaryCmdBuf != NULL);


    // Push a submittal, in order to signal the semaphores.
    VkSubmitInfo submitInfo             = { };
    VkPipelineStageFlags waitStages[]   = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
    submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount     = 1;
    submitInfo.commandBufferCount       = 1;
    submitInfo.pSignalSemaphores        = &signalSemaphore;
    submitInfo.waitSemaphoreCount       = 1;
    submitInfo.pWaitSemaphores          = &waitSemaphore;
    submitInfo.pCommandBuffers          = &primaryCmdBuf;
    submitInfo.pWaitDstStageMask        = waitStages;

    vkQueueSubmit(m_queue->get(), 1, &submitInfo, fence);

    return RecluseResult_Ok;
}


#define CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabled, available, feature) \
    if (available.feature) \
    { \
        enabled.feature = true; \
    } \
    else \
    { \
        R_VERBOSE(R_CHANNEL_VULKAN, "Feature %s can not be enabled for device creation! Not supported on this physical device!", #feature); \
    } 


R_INTERNAL
VkPhysicalDeviceFeatures checkEnableFeatures(VulkanAdapter* adapter)
{
    VkPhysicalDeviceFeatures enabledFeatures    = { };
    VkPhysicalDeviceFeatures availableFeatures  = adapter->getFeatures();

    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, geometryShader);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, fillModeNonSolid);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, imageCubeArray);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, tessellationShader);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, depthClamp);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, depthBiasClamp);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, depthBounds);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, logicOp);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, alphaToOne);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, samplerAnisotropy);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, wideLines);

    return enabledFeatures;
}


ResultCode VulkanDevice::initialize(VulkanAdapter* adapter, DeviceCreateInfo& info)
{
    R_ASSERT_FORMAT(adapter != NULL, "Adapter must not be NULL! Device creation will fail!");
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 

    VkDeviceCreateInfo createInfo                       = { };

    VkPhysicalDeviceFeatures features                   = checkEnableFeatures(adapter);
    VulkanInstance* pVc                                 = adapter->getInstance();
    std::vector<VkQueueFamilyProperties> queueFamilies  = adapter->getQueueFamilyProperties();
    std::vector<const char*> deviceExtensions           = adapter->queryAvailableDeviceExtensions(pVc->getRequestedDeviceFeatures());

    createInfo.sType                                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // Add swapchain extension capability.
    if (adapter->checkSupportsDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
    {
        // Query for swapchain creation.
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        m_supportsSwapchainCreation = true;
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

        queueFamily.flags               = queueFamProps.queueFlags;
        queueInfo.queueCount            = 0;
        queueInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

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
    createInfo.pEnabledFeatures         = &features;

    VkResult result = vkCreateDevice(adapter->get(), &createInfo, nullptr, &m_device);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create vulkan device!");   
        return -1;
    }

    m_adapter = adapter;

    createQueues();
    createDescriptorHeap();
    allocateMemCache();

    m_allocationManager = makeSmartPtr(new VulkanAllocationManager());
    ResultCode err = m_allocationManager->initialize(this);

    if (err != RecluseResult_Ok)
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to initialize the allocation manager!");
    }

    m_enabledFeatures = features;

    return 0;
}


void VulkanDevice::release(VkInstance instance)
{
    vkDeviceWaitIdle(m_device);
    
    DescriptorSets::clearDescriptorLayoutCache(this);
    m_allocationManager->release();

    destroyQueues();
    destroyDescriptorHeap();
    freeMemCache();
    ResourceViews::clearCache(this);
    RenderPasses::clearCache(this);
    ShaderPrograms::unloadAll(this);
    Pipelines::VertexLayout::unloadAll();
    Pipelines::clearPipelineCache(this);

    if (m_device != VK_NULL_HANDLE) 
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        R_DEBUG(R_CHANNEL_VULKAN, "Device Destroyed.");
    }
}


VkDeviceSize VulkanDevice::getNonCoherentSize() const
{
    return m_adapter->getProperties().limits.nonCoherentAtomSize;
}


GraphicsSwapchain* VulkanDevice::createSwapchain
    (
        const SwapchainCreateDescription& pDesc,
        void* windowHandle
    )
{
    R_ASSERT_FORMAT(m_supportsSwapchainCreation, "This device does not support swapchain creation. Be sure to use a physical device that does!!");
    VkSurfaceKHR surface                = getAdapter()->getInstance()->makeSurface(windowHandle); 
    VulkanQueue* pQueue                 = getPresentableQueue(surface);
    VulkanSwapchain* pSwapchain         = new VulkanSwapchain(pDesc, pQueue);
    VulkanInstance*   pNativeContext    = m_adapter->getInstance();

    ResultCode result = pSwapchain->build(this, windowHandle);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Swapchain failed to create");
        delete pSwapchain;
        return nullptr;
    }

    return pSwapchain;
}


VulkanQueue* VulkanDevice::getPresentableQueue(VkSurfaceKHR surface)
{
    VkQueueFlags flags = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
    VulkanQueue* pQueue = getQueue(flags);
    if (!pQueue->isPresentSupported(getAdapter(), surface))
    {
        // If the current queue doesn't support the given surface present, we need to find a new one.
        // TODO: We will need to figure out how to update other resources that might be using the old queue!!
        VulkanQueue queue = makeQueue((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT), true, surface);
        auto& iter = m_queues.find(flags);
        if (iter != m_queues.end())
        {
            iter->second.destroy();
            m_queues.erase(iter);
            m_queues.insert(std::make_pair(flags, queue));
            pQueue = &m_queues[flags];
        }
    }
    return pQueue;
}


ResultCode VulkanDevice::destroySwapchain(VulkanSwapchain* pSwapchain)
{
    ResultCode result = RecluseResult_Ok;

    if (!pSwapchain) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Null pointer exception with either pContext or pSwapchain.");   
        return RecluseResult_NullPtrExcept;
    }

    VulkanQueue* pPq = pSwapchain->getPresentationQueue();

    if (pPq) 
    {

        pPq->wait();

    }

    result = pSwapchain->release();

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to destroy vulkan swapchain!");
    } 
    else 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroyed vulkan swapchain...");
        
        delete pSwapchain;
    }


    return result;
}


ResultCode VulkanDevice::reserveMemory(const MemoryReserveDescription& desc)
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


ResultCode VulkanDevice::createQueues()
{
    // Lets create the primary queue.
    VulkanQueue queue;
    ResultCode result       = RecluseResult_Ok;
    queue = makeQueue((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));

    if (queue.get() == VK_NULL_HANDLE) 
    { 
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create main RHI queue!");
        result = RecluseResult_Failed;
    }
    else
    {
        m_queues[(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)] = queue;
    }

    queue = makeQueue(VK_QUEUE_COMPUTE_BIT);

    if (queue.get() == VK_NULL_HANDLE)
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Async Compute not supported. No queue was created for this...");
        result = RecluseResult_Failed;
    }
    else
    {
        m_queues[VK_QUEUE_COMPUTE_BIT] = queue;
    }

    return result;
}


VulkanQueue VulkanDevice::makeQueue(VkQueueFlags flags, Bool reuse, VkSurfaceKHR surfaceToPresent)
{
    U32 queueIndex          = 0xFFFFFFFF;
    VulkanQueue queue       = { };
    QueueFamily* pFamily    = nullptr;
    VulkanAdapter* pAdapter = getAdapter();

    for (U32 i = 0; i < m_queueFamilies.size(); ++i) 
    {
        QueueFamily& family = m_queueFamilies[i];

        // Passing the surface will mean we want a queue that supports it.
        if (surfaceToPresent && !pAdapter->checkSurfaceSupport(family.queueFamilyIndex, surfaceToPresent))
        {
            // If this queue family is not present supported, then check the next.
            continue;
        }

        if (family.flags & flags) 
        {
            if (!reuse)
            {
                pFamily             = &family;
                // Check if we can get a queue from this family.
                if (family.currentAvailableQueueIndex < family.maxQueueCount) 
                {
                    queueIndex          = family.currentAvailableQueueIndex++;
                    break;   
                }
            }
            else
            {
                // We reuse the same queue.
                queueIndex = family.currentAvailableQueueIndex;
                break;
            }
        }
    }

    if (queueIndex == 0xFFFFFFFF) 
    {
        // Can not find proper queue family, fails to create a queue!
        return queue;
    }

    queue = VulkanQueue(flags);

    ResultCode err = queue.initialize(this, pFamily, queueIndex);
    if (err != RecluseResult_Ok) 
    {
        queue.destroy();
    }

    return queue;
}


ResultCode VulkanDevice::destroyQueues()
{
    for (auto& iter : m_queues)
    {
        iter.second.destroy();
    }
    m_queues.clear();
    return RecluseResult_Ok;
}


ResultCode VulkanDevice::createResource(GraphicsResource** ppResource, const GraphicsResourceDescription& desc, ResourceState initState)
{
    VulkanResource* pResource = Resources::makeResource(this, desc, initState);
    *ppResource = pResource;
    return pResource ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode VulkanDevice::destroyResource(GraphicsResource* pResource)
{
    if (pResource) 
    {
        return Resources::releaseResource(this, pResource->getId());
    }

    return RecluseResult_Failed;
}


ResultCode VulkanContext::createCommandPools(U32 buffers)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Creating command pools...");

    VkCommandPoolCreateInfo poolIf                  = { };
    poolIf.sType                                    = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolIf.flags                                    = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkQueueFlags queueFlags                         = (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT); 
    ResultCode result                               = RecluseResult_Ok;
    poolIf.queueFamilyIndex = m_queue->getFamily()->queueFamilyIndex;
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
            R_ERROR(R_CHANNEL_VULKAN, "Failed to create command pool for queue family...");
            destroyCommandPools();
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


ResultCode VulkanContext::createPrimaryCommandList(VkQueueFlags flags)
{
    ResultCode result = RecluseResult_Ok;
    U32 queueFamilyIndex = m_queue->getFamily()->queueFamilyIndex;
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
        R_ERROR(R_CHANNEL_VULKAN, "Could not create CommandList...");

        m_primaryCommandList.release(this);

        return result;
    }
    return result;
}


ResultCode VulkanContext::destroyPrimaryCommandList()
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
    U32 currentBufferIndex = getCurrentFrameIndex();

    // Reset the current buffer's command pools.
    resetCommandPool(currentBufferIndex, true);
    DescriptorSets::clearDescriptorSetCache(this);

    const VulkanAllocationManager::Flags allocUpdate = (VulkanAllocationManager::Flag_SetFrameIndex | VulkanAllocationManager::Flag_Update);

    VulkanAllocationManager::UpdateConfig config;

    config.flags                = allocUpdate;
    config.frameIndex           = currentBufferIndex;
    config.garbageBufferCount   = m_bufferCount;

    m_pDevice->getAllocationManager()->update(config);
    
    // TODO: probably want to figure out a cleaner way of doing this.
    DescriptorSets::clearDescriptorSetCache(this, DescriptorSets::ClearCacheFlag_DescriptorPoolFastClear);
    resetBinds();
}


void VulkanContext::createContextFrames(U32 buffering)
{
    m_frameResources.resize(buffering);
    for (U32 i = 0; i < m_frameResources.size(); ++i) 
    {
        VulkanContextFrame frame = { };
        // Create fences with signalled bit, in order for the swapchain to properly 
        // wait on our fences, this should handle initial startup of application rendering, and not cause a block.
        VkFenceCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(m_pDevice->get(), &info, nullptr, &frame.fence);

        VkSemaphoreCreateInfo semaphoreInfo = { };
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.flags = 0;
        vkCreateSemaphore(m_pDevice->get(), &semaphoreInfo, nullptr, &frame.waitSemaphore);
        vkCreateSemaphore(m_pDevice->get(), &semaphoreInfo, nullptr, &frame.signalSemaphore);
        m_frameResources[i] = frame;
    }
}


void VulkanContext::destroyContextFrames()
{
    for (U32 i = 0; i < m_frameResources.size(); ++i) 
    {
        vkDestroyFence(m_pDevice->get(), m_frameResources[i].fence, nullptr);
        vkDestroySemaphore(m_pDevice->get(), m_frameResources[i].waitSemaphore, nullptr);
        vkDestroySemaphore(m_pDevice->get(), m_frameResources[i].signalSemaphore, nullptr);
    }
    m_frameResources.clear();
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
    UPtr alignedFlushAddress        = align(m_memCache.flush.pool->getBaseAddress(), pointerSizeBytes());
    UPtr alignedInvalidAddress      = align(m_memCache.invalid.pool->getBaseAddress(), pointerSizeBytes());

    m_memCache.flush.allocator->initialize(alignedFlushAddress, cacheSizeBytes);
    m_memCache.invalid.allocator->initialize(alignedInvalidAddress, cacheSizeBytes);
    m_memCache.m_flushCs.initialize();
    m_memCache.m_invalidCs.initialize();
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

    m_memCache.m_flushCs.release();
    m_memCache.m_invalidCs.release();
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
        R_ERROR(R_CHANNEL_VULKAN, "Failed to flush memory ranges...");    
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
        R_ERROR(R_CHANNEL_VULKAN, "Failed to invalidate memory ranges...");    
    }

    m_memCache.invalid.allocator->reset();
}


void VulkanDevice::pushFlushMemoryRange(const VkMappedMemoryRange& mappedRange)
{
    VkDeviceSize atomSz     = getNonCoherentSize();
    // Alignment of 0 since we want to pack these ranges together. We hope that 
    // the api struct itself will be aligned properly.
    ResultCode result          = RecluseResult_Ok;
    UPtr address = 0ull;

    {
        ScopedCriticalSection _(m_memCache.m_flushCs);
        address = m_memCache.flush.allocator->allocate(sizeof(VkMappedMemoryRange), 0);
        result = m_memCache.flush.allocator->getLastError();
    } 

    if (result != RecluseResult_Ok) 
    {
        if (result == RecluseResult_OutOfMemory) 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Memory flush cache out of memory!");        
        } 
        else 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Failed to push flush memory range!");        
        }
    
        // Possible null ptr returned, breaking off.
        return;
    }

    VkMappedMemoryRange* pRange = (VkMappedMemoryRange*)address;
    *pRange                     = mappedRange;

    // We need to align on nonCoherentAtomSize, as spec states it must be a multiple of this.
    pRange->size                = align(mappedRange.size, atomSz);
}


void VulkanDevice::pushInvalidateMemoryRange(const VkMappedMemoryRange& mappedRange)
{
    VkDeviceSize atomSz     = getNonCoherentSize();
    // Alignement of 0 since we want to pack these ranges together. We hope that the api struct itself will be aligned properly.
    ResultCode result          = RecluseResult_Ok;
    UPtr address = 0ull;

    {
        ScopedCriticalSection _(m_memCache.m_invalidCs);
        address = m_memCache.invalid.allocator->allocate(sizeof(VkMappedMemoryRange), 0);
        result = m_memCache.invalid.allocator->getLastError();
    }

    if (result != RecluseResult_Ok) 
    {
        if (result == RecluseResult_OutOfMemory) 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Memory flush cache out of memory!");        

        } 
        else 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Failed to push flush memory range!");        
        }

        // Possible nullptr, break off.
        return;
    
    }

    VkMappedMemoryRange* pRange = (VkMappedMemoryRange*)address;
    *pRange                     = mappedRange;

    pRange->size                = align(mappedRange.size, atomSz);
}


ResultCode VulkanContext::wait()
{
    m_queue->wait();
    return RecluseResult_Ok;
}


ResultCode VulkanDevice::createSampler(GraphicsSampler** ppSampler, const SamplerDescription& desc)
{
    VulkanSampler* pVSampler    = ResourceViews::makeSampler(this, desc);
    if (!pVSampler) return RecluseResult_Failed;
    *ppSampler = pVSampler;
    return RecluseResult_Ok;
}

ResultCode VulkanDevice::destroySampler(GraphicsSampler* pSampler)
{
    if (!pSampler) return RecluseResult_NullPtrExcept;
    return ResourceViews::releaseSampler(this, pSampler->getId());
}


ResultCode VulkanDevice::loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition)
{
    if (ShaderPrograms::isProgramCached(program, permutation))
    {
        return RecluseResult_NeedsUpdate;
    }
    return ShaderPrograms::loadNativeShaderProgramPermutation(this, program, permutation, definition);
}


ResultCode VulkanDevice::unloadShaderProgram(ShaderProgramId program)
{
    if (!ShaderPrograms::isProgramCached(program))
    {
        return RecluseResult_Ok;
    }
    return ShaderPrograms::unloadProgram(this, program);
}


void VulkanDevice::unloadAllShaderPrograms()
{
    // Clear all shader programs and associated pipeline states as well.
    ShaderPrograms::unloadAll(this);
    Pipelines::clearPipelineCache(this);
}


Bool VulkanDevice::makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout)
{
    ResultCode result = Pipelines::VertexLayout::make(id, layout);
    return (result == RecluseResult_Ok || result == RecluseResult_AlreadyExists);
}


Bool VulkanDevice::destroyVertexLayout(VertexInputLayoutId id)
{
    ResultCode result = Pipelines::VertexLayout::unloadLayout(id);
    return (result == RecluseResult_Ok || result == RecluseResult_NotFound);
}


GraphicsContext* VulkanDevice::createContext()
{
    m_allocatedContexts.reserve(kMaxGraphicsContexts);
    if (m_allocatedContexts.size() >= kMaxGraphicsContexts)
    {
        R_ERROR(R_CHANNEL_VULKAN, "Reached maximum allowable graphics contexts to create!");
        return nullptr;
    }

    VulkanContext* pContext = new VulkanContext(this, getQueue(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT));
    m_allocatedContexts.push_back(pContext);
    return pContext;
}


ResultCode VulkanDevice::releaseContext(GraphicsContext* pContext)
{
    VulkanContext* pVc = static_cast<VulkanContext*>(pContext);
    pVc->release();
    delete pVc;
    return RecluseResult_Ok;
}


void VulkanDevice::copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, const CopyBufferRegion* regions, U32 numRegions)
{
    getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)->copyBufferRegions(dst, src, regions, numRegions);
}


void VulkanDevice::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)->copyResource(dst, src);
}


ResultCode VulkanDevice::destroySwapchain(GraphicsSwapchain* pSwapchain)
{
    VulkanSwapchain* vulkanSwapchain = pSwapchain->castTo<VulkanSwapchain>();
    vulkanSwapchain->release();
    delete pSwapchain;
    return RecluseResult_Ok;
}
} // Recluse