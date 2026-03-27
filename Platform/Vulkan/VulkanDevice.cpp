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
namespace Vulkan {

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
    config.flags = VulkanAllocationManager::Flag_GarbageResize;
    config.garbageBufferCount = bufferCount;
    config.frameIndex = m_currentContextFrameIndex;
    allocManager->update(config);

    m_tableArena.preAllocate(sizeof(LinearAllocator) + R_MB(1));
#if defined(RECLUSE_EXPERIMENTAL)
    m_tableAllocator = new (reinterpret_cast<void*>(m_tableArena.getBaseAddress())) LinearAllocator();
    m_tableAllocator->initialize(m_tableArena.getPtrAddressAt(sizeof(LinearAllocator)),
        m_tableArena.getTotalSizeBytes() - sizeof(LinearAllocator));
#endif
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

    m_tableArena.release();

    // Ensure we no longer have any buffers.
    m_bufferCount = 0;
    m_currentContextFrameIndex = 0;
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
    incrementContextFrameIndex();
    VulkanContextFrame& contextFrame    = getContextFrame(getCurrentFrameIndex());
    contextFrame.flags                  = ContextFrameFlag_None;
    VkFence frameFence                  = contextFrame.fence;

    // We need to wait for our fences, before we can begin to reset resources.
    vkWaitForFences(m_pDevice->get(), 1, &frameFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_pDevice->get(), 1, &frameFence);

    if (Vulkan::targetApiVersion >= R_VULKAN_MAKE_API_VERSION(1, 2, 0))
    {
        // Reset our queries.
        contextFrame.timestampQuery.reset(m_pDevice->get());
        contextFrame.occlusionQuery.reset(m_pDevice->get());
    }
    
    prepare();

    m_primaryCommandList.use(getCurrentFrameIndex());
    m_primaryCommandList.reset();
    m_primaryCommandList.begin();

    // Clear the temporary buffer.
    contextFrame.temporaryBufferAllocator.clear();

    if (Vulkan::targetApiVersion < R_VULKAN_MAKE_API_VERSION(1, 2, 0))
    {
        contextFrame.timestampQuery.resetLegacy(m_primaryCommandList.get());
        contextFrame.occlusionQuery.resetLegacy(m_primaryCommandList.get());
    }

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
    // We need to make sure queue is actually available, otherwise this will crash.
    R_ASSERT(m_graphicsQueue);
    // If we were granted a compute queue, we support async compute.
    return (m_computeQueue != nullptr) && (m_computeQueue->getFamily()->queueFamilyIndex != m_graphicsQueue->getFamily()->queueFamilyIndex); 
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
    RenderPasses::checkLruCache(m_pDevice);
    Pipelines::clean(m_pDevice);
}




ResultCode VulkanContext::submitFinalCommandBuffer(VkCommandBuffer commandBuffer)
{
    FrameIndex currentFrameIndex        = getCurrentFrameIndex();
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
    // If no swapchain image wait, we don't use semaphore signalling.
    VkSubmitInfo submitInfo             = { };
    VkPipelineStageFlags waitStages[]   = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

    //const Bool swapchainQueued          = (contextFrame.flags & ContextFrameFlag_SwapchainQueued);

    submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount       = 1;
    submitInfo.signalSemaphoreCount     = signalSemaphore ? 1 : 0;
    submitInfo.pSignalSemaphores        = &signalSemaphore;
    submitInfo.waitSemaphoreCount       = waitSemaphore ? 1 : 0;
    submitInfo.pWaitSemaphores          = &waitSemaphore;
    submitInfo.pCommandBuffers          = &primaryCmdBuf;
    submitInfo.pWaitDstStageMask        = waitStages;

    vkQueueSubmit(m_graphicsQueue->get(), 1, &submitInfo, fence);

    // clear the semaphores.
    contextFrame.signalSemaphore = VK_NULL_HANDLE;
    contextFrame.waitSemaphore = VK_NULL_HANDLE;

    return RecluseResult_Ok;
}


#define CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabled, available, feature) \
    if (available.features2.features.feature) \
    { \
        enabled.features2.features.feature = true; \
    } \
    else \
    { \
        R_VERBOSE(R_CHANNEL_VULKAN, "Feature %s can not be enabled for device creation! Not supported on this physical device!", #feature); \
    } 


R_INTERNAL
PhysicalDeviceFeaturesInfo checkEnableFeatures(VulkanAdapter* adapter)
{
    PhysicalDeviceFeaturesInfo enabledFeatures   = { };
    PhysicalDeviceFeaturesInfo availableFeatures  = adapter->getFeatures2();

    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, geometryShader);
    CHECK_AND_ENABLE_FEATURE_IF_AVAILABLE(enabledFeatures, availableFeatures, tessellationShader);
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

    if (availableFeatures.hostQueryResetFeatures.hostQueryReset)
    {
        enabledFeatures.hostQueryResetFeatures.hostQueryReset = true;
    }
    else
    {
        enabledFeatures.hostQueryResetFeatures.hostQueryReset = false;
    }

#ifdef VK_NV_mesh_shader
    if (adapter->checkSupportsDeviceExtension(VK_NV_MESH_SHADER_EXTENSION_NAME) && availableFeatures.meshShaderFeaturesNV.meshShader)
    {
        enabledFeatures.meshShaderFeaturesNV.meshShader = VK_TRUE;
        enabledFeatures.meshShaderFeaturesNV.taskShader = VK_TRUE;
    }
#endif

#ifdef VK_EXT_mesh_shader
    if (adapter->checkSupportsDeviceExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME) && availableFeatures.meshShaderFeaturesEXT.meshShader)
    {
        enabledFeatures.meshShaderFeaturesEXT.meshShader = VK_TRUE;
        enabledFeatures.meshShaderFeaturesEXT.taskShader = VK_TRUE;
        // Disable these for now.
        enabledFeatures.meshShaderFeaturesEXT.multiviewMeshShader = VK_FALSE;
        enabledFeatures.meshShaderFeaturesEXT.primitiveFragmentShadingRateMeshShader = VK_FALSE;
    }
#endif

    return enabledFeatures;
}


ResultCode VulkanDevice::initialize(VulkanAdapter* adapter, DeviceCreateInfo& info, U32 deviceId)
{
    R_ASSERT_FORMAT(adapter != NULL, "Adapter must not be NULL! Device creation will fail!");
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; 

    VkDeviceCreateInfo createInfo                       = { };
    
    PhysicalDeviceFeaturesInfo features                 = checkEnableFeatures(adapter);
   
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
    createInfo.pEnabledFeatures         = nullptr; //&features.features2.features;
    createInfo.pNext                    = &features.features2; // We instead need to pass features2 to pNext, which requires pEnabledFeatures to be NULL.

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
    setDeviceId(deviceId);

    if (err != RecluseResult_Ok)
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to initialize the allocation manager!");
    }

    m_enabledFeatures = features.features2.features;

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
    Pipelines::VertexLayout::unloadAll(getDeviceId());
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


ResultCode VulkanDevice::createSwapchain
    (
        const SwapchainCreateDescription& pDesc,
        void* windowHandle,
        GraphicsSwapchain** outSwapchain
    )
{
    R_ASSERT_FORMAT(windowHandle != NULL, "Window handle was null when creating swapchain!");
    R_ASSERT_FORMAT(m_supportsSwapchainCreation, "This device does not support swapchain creation. Be sure to use a physical device that does!!");

    VkSurfaceKHR surface                = getAdapter()->getInstance()->makeSurface(windowHandle); 

    if (surface == VK_NULL_HANDLE)
        return RecluseResult_Failed;

    VulkanQueue* pQueue                 = getPresentableQueue(surface);
    VulkanSwapchain* pSwapchain         = new VulkanSwapchain(pDesc, pQueue);
    VulkanInstance*   pNativeContext    = m_adapter->getInstance();

    ResultCode result = pSwapchain->build(this, windowHandle);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Swapchain failed to create");
        delete pSwapchain;
        return result;
    }
    *outSwapchain = pSwapchain;
    return result;
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
            desc.bufferPools[ResourceMemoryUsage_CpuVisible], 
            desc.bufferPools[ResourceMemoryUsage_GpuOnly], 
            desc.texturePoolGPUOnly
        );

    R_DEBUG
        (
            R_CHANNEL_VULKAN, 
            "Total available memory (GB):\n\tDevice: %f\n\tHost: %f", 
            F32(desc.bufferPools[ResourceMemoryUsage_GpuOnly] + desc.texturePoolGPUOnly) / R_1GB,
            F32(desc.bufferPools[ResourceMemoryUsage_CpuVisible]) / R_1GB
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


ResultCode VulkanDevice::createResource(GraphicsResource** ppResource, const GraphicsResourceDescription& desc, ResourceState initState, GraphicsClearColor* clearColor)
{
    VulkanResource* pResource = Resources::makeResource(this, desc, initState, clearColor);
    *ppResource = pResource;
    return pResource ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode VulkanDevice::destroyResource(GraphicsResource* pResource, Bool immediate)
{
    if (pResource) 
    {
        return Resources::releaseResource(this, pResource->getId(), immediate);
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
    poolIf.queueFamilyIndex = m_graphicsQueue->getFamily()->queueFamilyIndex;
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
    U32 queueFamilyIndex = m_graphicsQueue->getFamily()->queueFamilyIndex;
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


void VulkanContext::resetCommandPool(FrameIndex bufferIdx, Bool resetAllResources)
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
    FrameIndex currentBufferIndex = getCurrentFrameIndex();

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
    RenderPasses::updateTick(m_pDevice);
    Pipelines::update(m_pDevice->getDeviceId());

#if defined(RECLUSE_EXPERIMENTAL)
    m_tableAllocator->reset();
#endif
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

        frame.timestampQuery.initialize(m_pDevice->get(), VK_QUERY_TYPE_TIMESTAMP, 128);
        frame.occlusionQuery.initialize(m_pDevice->get(), VK_QUERY_TYPE_OCCLUSION, 128);

        frame.temporaryBufferAllocator.initialize(m_pDevice);
        frame.signalSemaphore = VK_NULL_HANDLE;
        frame.waitSemaphore = VK_NULL_HANDLE;

        m_frameResources[i] = frame;
    }

    m_currentContextFrameIndex = -1;
}


void VulkanContext::destroyContextFrames()
{
    for (U32 i = 0; i < m_frameResources.size(); ++i) 
    {
        vkDestroyFence(m_pDevice->get(), m_frameResources[i].fence, nullptr);

        m_frameResources[i].timestampQuery.release(m_pDevice->get());
        m_frameResources[i].occlusionQuery.release(m_pDevice->get());

        m_frameResources[i].temporaryBufferAllocator.release();
    }
    m_frameResources.clear();
}


GraphicsQuery VulkanContext::beginQuery(GraphicsQueryType type)
{
    GraphicsQuery query = { };
    VulkanContextFrame& contextFrame = getContextFrame(getCurrentFrameIndex());
    VulkanQueryManager* manager = nullptr;
    switch (type)
    {
        case GraphicsQueryType_Occlusion: manager = &contextFrame.occlusionQuery; break;
        case GraphicsQueryType_Timestamp: manager = &contextFrame.timestampQuery; break;
        default: break;
    }

    if (manager)
    {
        VulkanQueryManager::Index index = manager->beginQuery(m_primaryCommandList.get());
        query = { index, type };
    }
    return query;
}


void VulkanContext::endQuery(const GraphicsQuery& query)
{
    if (!query.isValid())
        return;
    
    VulkanContextFrame& contextFrame = getContextFrame(getCurrentFrameIndex());
    VulkanQueryManager* manager = nullptr;
    switch (query.getType())
    {
        case GraphicsQueryType_Occlusion: manager = &contextFrame.occlusionQuery; break;
        case GraphicsQueryType_Timestamp: manager = &contextFrame.timestampQuery; break;
        default: break;
    }

    if (manager)
        manager->endQuery(m_primaryCommandList.get(), query);
}


void VulkanContext::beginLabel(const char* label, const Math::Float4& color)
{
    VkCommandBuffer cmdlist = m_primaryCommandList.get();
    VulkanInstance* instance = m_pDevice->getAdapter()->getInstance();
    if (instance->supportsDebugMarking())
    {
        VkDebugUtilsLabelEXT labelDesc = { };
        labelDesc.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;

        labelDesc.pLabelName = label;        

        labelDesc.color[0] = color[0];
        labelDesc.color[1] = color[1];
        labelDesc.color[2] = color[2];
        labelDesc.color[3] = color[3];

        pfn_vkCmdBeginDebugUtilsLabelEXT(cmdlist, &labelDesc);
    }
}


void VulkanContext::endLabel()
{
    VkCommandBuffer cmdlist = m_primaryCommandList.get();
    VulkanInstance* instance = m_pDevice->getAdapter()->getInstance();
    if (instance->supportsDebugMarking())
    {
        pfn_vkCmdEndDebugUtilsLabelEXT(cmdlist);
    }
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


void VulkanDevice::MemoryManager::initialize()
{
    U64 cacheSizeBytes  = align(sizeof(VkMappedMemoryRange) * 128ULL, pointerSizeBytes());
    pool                = std::make_unique<MemoryPool>(cacheSizeBytes);
    allocator           = std::make_unique<LinearAllocator>();

    UPtr alignedAddress = align(pool->getBaseAddress(), pointerSizeBytes());

    allocator->initialize(alignedAddress, cacheSizeBytes);
    cs.initialize();
};


void VulkanDevice::MemoryManager::free()
{
    if (allocator)
    {
        allocator->cleanUp();
        allocator.release();
    }

    if (pool)
    {
        pool.release();
    }

    cs.release();
}


void VulkanDevice::allocateMemCache()
{
    // TODO: In the future, we might need to consider multithreading cases, although
    //       I don't think we will have more than one main rendering thread.
    //U64 cacheSizeBytes              = align(sizeof(VkMappedMemoryRange) * 128ull, pointerSizeBytes());
    //m_memCache.flush.pool           = new MemoryPool(cacheSizeBytes);
    //m_memCache.invalid.pool         = new MemoryPool(cacheSizeBytes);
    //m_memCache.flush.allocator      = new LinearAllocator();
    //m_memCache.invalid.allocator    = new LinearAllocator();
    //UPtr alignedFlushAddress        = align(m_memCache.flush.pool->getBaseAddress(), pointerSizeBytes());
    //UPtr alignedInvalidAddress      = align(m_memCache.invalid.pool->getBaseAddress(), pointerSizeBytes());

    //m_memCache.flush.allocator->initialize(alignedFlushAddress, cacheSizeBytes);
    //m_memCache.invalid.allocator->initialize(alignedInvalidAddress, cacheSizeBytes);
    //m_memCache.m_flushCs.initialize();
    //m_memCache.m_invalidCs.initialize();
    m_memCache.flush.initialize();
    m_memCache.invalid.initialize();
}


void VulkanDevice::freeMemCache()
{
    //if (m_memCache.flush.allocator) 
    //{
    //    m_memCache.flush.allocator->cleanUp();
    //    delete m_memCache.flush.allocator;
    //    m_memCache.flush.allocator = nullptr;        
    //}
    //
    //if (m_memCache.flush.pool) 
    //{
    //    delete m_memCache.flush.pool; 
    //    m_memCache.flush.pool = nullptr;   
    //}

    //if (m_memCache.invalid.allocator) 
    //{
    //    m_memCache.invalid.allocator->cleanUp();
    //    delete m_memCache.invalid.allocator;
    //    m_memCache.invalid.allocator = nullptr;   
    //}

    //if (m_memCache.invalid.pool) 
    //{
    //    delete m_memCache.invalid.pool;
    //    m_memCache.invalid.pool = nullptr;
    //}

    //m_memCache.m_flushCs.release();
    //m_memCache.m_invalidCs.release();

    m_memCache.flush.free();
    m_memCache.invalid.free();
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
        ScopedCriticalSection _(m_memCache.flush.cs);
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
        ScopedCriticalSection _(m_memCache.invalid.cs);
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
    m_graphicsQueue->wait();
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
    if (definition.intermediateCode != ShaderIntermediateCode_Spirv)
    {
        R_ERROR(R_CHANNEL_VULKAN, "Unable to load ShaderProgramDefinition! Compiled shaders are not SPIR-V!!");
        return RecluseResult_Failed;
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
    if (id == VertexInputLayout::VertexLayout_Null)
    {
        R_ERROR(R_CHANNEL_VULKAN, "Can not make vertex layouts with id of %d! This is reserved for null arguments.", VertexInputLayout::VertexLayout_Null);
        return false;
    } 
    ResultCode result = Pipelines::VertexLayout::make(getDeviceId(), id, layout);
    return (result == RecluseResult_Ok || result == RecluseResult_AlreadyExists);
}


Bool VulkanDevice::destroyVertexLayout(VertexInputLayoutId id)
{
    ResultCode result = Pipelines::VertexLayout::unloadLayout(getDeviceId(), id);
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
    if (!pSwapchain)
        return RecluseResult_NullPtrExcept;
    VulkanSwapchain* vulkanSwapchain = pSwapchain->castTo<VulkanSwapchain>();
    vulkanSwapchain->release();
    delete pSwapchain;
    return RecluseResult_Ok;
}


VkMemoryRequirements VulkanDevice::getBufferMemoryRequirements(VkBuffer buffer) const
{
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(get(), buffer, &requirements);
    return requirements;
}


VkMemoryRequirements VulkanDevice::getImageMemoryRequirements(VkImage image) const
{
    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(get(), image, &requirements);
    return requirements;
}


void VulkanContext::VulkanShaderProgramBinder::obtainShaderProgramFromCache()
{
    cachedProgram = ShaderPrograms::obtainShaderProgram(getProgramId(), getPermutationId());
    if (cachedProgram)
    {
        reflectionCache = ShaderPrograms::obtainProgramReflection(getProgramId(), getPermutationId());
        if (reflectionCache)
        {
            // Reserve the needed size of the shader program.
            R_ASSERT(currentState().m_boundDescriptorSetStructures.size() >= reflectionCache->sets.size());
            R_ASSERT(currentState().m_boundPerSet.size() >= reflectionCache->sets.size());
            currentState().m_numSets = reflectionCache->sets.size();
            for (U32 set = 0; set < reflectionCache->sets.size(); ++set)
            {
                currentState().m_boundDescriptorSetStructures[set].key.value.constantBuffers    = (U16)reflectionCache->sets[set].numCbvs;
                currentState().m_boundDescriptorSetStructures[set].key.value.srvs               = (U16)reflectionCache->sets[set].numSrvs;
                currentState().m_boundDescriptorSetStructures[set].key.value.uavs               = (U16)reflectionCache->sets[set].numUavs;
                currentState().m_boundDescriptorSetStructures[set].key.value.samplers           = (U16)reflectionCache->sets[set].numSamplers;
            }
        }
    }
}


ResourceView VulkanContext::allocateConstantBuffer(U32 cbSizeBytes, void* dat)
{
    VulkanContextFrame& contextFrame = getContextFrame(getCurrentFrameIndex());
    BufferTemporaryAllocator::Result block{};
    ResultCode result = contextFrame.temporaryBufferAllocator.allocate(&block, ResourceMemoryUsage_CpuToGpu, cbSizeBytes);

    if (result == RecluseResult_OutOfMemory)
    {
        R_ASSERT("Out of constant buffer memory!!");
        return {};
    }
    
    if (dat)
        memcpy((void*)block.memPtr, dat, cbSizeBytes);
    else
        R_WARN(R_CHANNEL_VULKAN, "No data copied to this resource! Will be null.");

    ResourceView view{};
    memcpy(&view, &block.bufferView, sizeof(ResourceView));
    return view;
}


PFN_vkVoidFunction VulkanDevice::getProcAddr(const char* procName)
{
    return (PFN_vkVoidFunction)vkGetDeviceProcAddr(get(), procName);
}


void VulkanDevice::loadFunctions()
{
    
}

Bool VulkanDevice::isResourceFormatSupported(ResourceFormat format)
{
    VkFormatProperties properties = getAdapter()->getFormatProperties(Vulkan::getVulkanFormat(format));
    // Check if any features are supported on the format.
    if (properties.linearTilingFeatures || properties.optimalTilingFeatures)
        return true;
    return false;
}


void VulkanContext::registerFrameSemaphores(VkSemaphore wait, VkSemaphore signal)
{
    VulkanContextFrame& frame = getContextFrame(getCurrentFrameIndex());
    frame.signalSemaphore = signal;
    frame.waitSemaphore = wait;
}
} // Vulkan
} // Recluse