// 
#pragma once

#include "VulkanInstance.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanShaderCache.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include <vector>
#include <list>

namespace Recluse {


class VulkanAdapter;
class VulkanQueue;
class VulkanSwapchain;
class VulkanAllocator;
class VulkanDescriptorManager;
class VulkanPrimaryCommandList;
class VulkanDevice;
class VulkanSwapchain;
class Allocator;
class MemoryPool;
struct DeviceCreateInfo;


struct QueueFamily 
{
    U32                                     maxQueueCount;
    U32                                     queueFamilyIndex;
    U32                                     currentAvailableQueueIndex;
    VkQueueFlags                            flags;
    B32                                     isPresentSupported;
    std::vector<VkCommandPool>              commandPools;
};


class VulkanContext : public GraphicsContext
{
public:
    VulkanContext(VulkanDevice* pDevice, U32 buffers)
        : m_bufferCount(buffers)
        , m_currentBufferIndex(0)
        , m_pDevice(pDevice) 
    { 
        initialize(); 
    }
    
    void release();

    GraphicsCommandList* getCommandList() override;

    void begin() override;
    void end() override;

    inline U32 getBufferCount() const { return m_bufferCount; }
    inline U32 getCurrentBufferIndex() const { return m_currentBufferIndex; }
    inline VkFence getCurrentFence() const { return m_fences[m_currentBufferIndex]; }
    ErrType createCommandList(VulkanPrimaryCommandList** pList, VkQueueFlags flags);
    ErrType destroyCommandList(VulkanPrimaryCommandList* pList);

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    ErrType copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
        CopyBufferRegion* pRegions, U32 numRegions) override;

    // Increment the buffer index.
    inline void incrementBufferIndex() {
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount;
    }

    GraphicsDevice* getDevice() override;

    ErrType wait() override;

private:
    void prepare();
    void initialize();
    void destroyFences();
    void createFences(U32 buffered);



    // buffer count 
    U32                         m_bufferCount;
    U32                         m_currentBufferIndex;
    std::vector<VkFence>        m_fences;
    VulkanPrimaryCommandList*          m_pPrimaryCommandList;
    VulkanDevice*               m_pDevice;
};


class VulkanDevice : public GraphicsDevice 
{
public:
    VulkanDevice()
        : m_device(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_windowHandle(nullptr)
        , m_adapter(nullptr)
        , m_pDescriptorManager(nullptr)
        , m_pGraphicsQueue(nullptr)
        , m_properties({ })
        , m_memCache({ })
        , m_swapchain(nullptr) 
        , m_context(nullptr)
    { 
        for (U32 i = 0; i < ResourceMemoryUsage_Count; ++i) 
        { 
            m_bufferPool[i].memory = VK_NULL_HANDLE;
            m_imagePool[i].memory = VK_NULL_HANDLE;
            m_bufferAllocators[i] = nullptr;
            m_imageAllocators[i] = nullptr;
        }
    }

    ErrType initialize(VulkanAdapter* iadapter, DeviceCreateInfo& info);
    VulkanQueue* getBackbufferQueue() { return m_pGraphicsQueue; }
    ErrType createQueues();
    const std::vector<QueueFamily>& getQueueFamilies() const { return m_queueFamilies; }    
    ErrType createCommandPools(U32 buffered);
    void destroyCommandPools();

    ErrType createQueue(VulkanQueue** ppQueue, VkQueueFlags flags, B32 isPresentable);
    ErrType destroyQueues();

    ErrType createSwapchain(VulkanSwapchain** ppSwapchain, 
        const SwapchainCreateDescription& pDesc, VulkanQueue* pPresentationQueue);

    GraphicsContext* getContext() override { return m_context; }

    ErrType destroySwapchain(VulkanSwapchain* pSwapchain);
    ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc, ResourceState initState) override;
    ErrType createDescriptorSetLayout(DescriptorSetLayout** ppLayout, const DescriptorSetLayoutDesc& desc) override;
    ErrType createDescriptorSet(DescriptorSet** ppDescriptorSet, DescriptorSetLayout* pLayout) override;
    ErrType createGraphicsPipelineState(PipelineState** ppPipelineState, const GraphicsPipelineStateDesc& desc) override;
    ErrType createRenderPass(RenderPass** ppRenderPass, const RenderPassDesc& desc) override;
    ErrType createSampler(GraphicsSampler** ppSampler, const SamplerCreateDesc& desc) override;
    ErrType destroySampler(GraphicsSampler* pSampler) override;
    ErrType destroyResource(GraphicsResource* pResource) override;
    ErrType destroyDescriptorSetLayout(DescriptorSetLayout* pLayout) override;
    ErrType createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc) override;
    ErrType createComputePipelineState(PipelineState** ppPipelineState, const ComputePipelineStateDesc& desc) override;
    void release(VkInstance instance);
    ErrType destroyPipelineState(PipelineState* pPipelineState) override;
    ErrType destroyResourceView(GraphicsResourceView* pResourceView) override;
    ErrType destroyDescriptorSet(DescriptorSet* pSet) override;
    ErrType destroyRenderPass(RenderPass* pRenderPass) override;
    void updateAllocators(const VulkanAllocator::UpdateConfig& config);

    VkDevice operator()() {
        return m_device;
    }

    VkDevice get() const {
        return m_device;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    VulkanAdapter* getAdapter() const { return m_adapter; }

    VulkanAllocator* getBufferAllocator(ResourceMemoryUsage usage) const
        { return m_bufferAllocators[usage]; }

    VulkanAllocator* getImageAllocator(ResourceMemoryUsage usage) const 
        { return m_imageAllocators[usage]; }

    VulkanDescriptorManager* getDescriptorHeap() { return m_pDescriptorManager; }
    
    ShaderCache* getShaderCache() { return &m_cache; }

    // Invalidate resources when reading back from GPU.
    void pushInvalidateMemoryRange(const VkMappedMemoryRange& mappedRange);

    // Flush the resource memory range when writing from CPU to GPU.
    void pushFlushMemoryRange(const VkMappedMemoryRange& mappedRange);

    void flushAllMappedRanges();
    void invalidateAllMappedRanges();

    VkDeviceSize getNonCoherentSize() const { return m_properties.limits.nonCoherentAtomSize; }

    GraphicsSwapchain* getSwapchain() override;

    Bool hasSurface() const { return m_surface != VK_NULL_HANDLE; }

private:

    ErrType createSurface(VkInstance instance, void* handle);
    void allocateMemCache();
    void createDescriptorHeap();


    void destroyDescriptorHeap();
    void freeMemCache();

    VulkanAdapter* m_adapter;

    VkDevice m_device;
    VkSurfaceKHR m_surface;
    struct 
    {
        struct 
        {
            MemoryPool* pool;
            Allocator* allocator;
        } invalid;

        struct 
        {
            MemoryPool* pool;
            Allocator* allocator;
        } flush;

    } m_memCache;

    VulkanMemoryPool m_bufferPool       [ResourceMemoryUsage_Count];
    VulkanMemoryPool m_imagePool        [ResourceMemoryUsage_Count];
    VulkanAllocator* m_bufferAllocators [ResourceMemoryUsage_Count];
    VulkanAllocator* m_imageAllocators  [ResourceMemoryUsage_Count];

    VulkanDescriptorManager*    m_pDescriptorManager;
    ShaderCache                 m_cache;
    VulkanSwapchain*            m_swapchain;
    VulkanContext*              m_context;
    std::vector<QueueFamily>    m_queueFamilies;
    std::vector<VkCommandPool>  m_commandPools;
    VulkanQueue*                m_pGraphicsQueue;
    
    void* m_windowHandle;
    
    // Adapter properties.
    VkPhysicalDeviceProperties m_properties;
};
} // Recluse