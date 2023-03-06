// 
#pragma once

#include "VulkanInstance.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanShaderCache.hpp"
#include "VulkanResource.hpp"
#include "VulkanObjects.hpp"
#include "VulkanViews.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "VulkanCommandList.hpp"
#include <vector>
#include <list>

namespace Recluse {


class VulkanAdapter;
class VulkanQueue;
class VulkanSwapchain;
class VulkanDescriptorManager;
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
        , m_boundRenderPass(nullptr)
        , m_pDevice(pDevice) 
    { 
        initialize(); 
    }
    
    void                release();
    void                begin() override;
    void                end() override;

    inline U32          getBufferCount() const { return m_bufferCount; }
    inline U32          getCurrentBufferIndex() const { return m_currentBufferIndex; }
    inline VkFence      getCurrentFence() const { return m_fences[m_currentBufferIndex]; }
    ErrType             createPrimaryCommandList(VkQueueFlags flags);
    ErrType             destroyPrimaryCommandList();

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    void                copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    void                copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
                            CopyBufferRegion* pRegions, U32 numRegions) override;

    // Increment the buffer index.
    inline void         incrementBufferIndex() {
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount;
    }

    GraphicsDevice*     getDevice() override;

    VulkanPrimaryCommandList* getPrimaryCommandList() { return &m_primaryCommandList; }

    ErrType             wait() override;

    VkRenderPass        getRenderPass();

    void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) override;
    void clearDepthStencil(F32 clearDepth, U8 clearStencil, const Rect& rect) override;

    void drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) override;
    void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) override;
    void bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) override;
    void drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance) override;

    void drawInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride) override;
    void drawIndexedInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride) override;
    void dispatchIndirect(GraphicsResource* pParams, U64 offset) override;

    void setViewports(U32 numViewports, Viewport* pViewports) override;
    void setScissors(U32 numScissors, Rect* pRects) override;
    void dispatch(U32 x, U32 y, U32 z) override;

    void transition(GraphicsResource* pResource, ResourceState dstState) override;

    void bindShaderResources(ShaderType type, U32 offset, U32 count, GraphicsResourceView** ppResources) override;
    void bindUnorderedAccessViews(ShaderType type, U32 offset, U32 count, GraphicsResourceView** ppResources) override;
    void bindConstantBuffers(ShaderType type, U32 offset, U32 count, GraphicsResource** ppResources) override;
    void bindRenderTargets(U32 count, GraphicsResourceView** ppResources, GraphicsResourceView* pDepthStencil) override;
    void bindSamplers(ShaderType type, U32 count, GraphicsSampler** ppSampler) override;
    void bindRasterizerState(const RasterState& state) override { }
    void bindBlendState(const BlendState& state) override { currentState().m_pipelineStructure.state.graphics.blendState = state; markPipelineDirty(); }
    void setTopology(PrimitiveTopology topology) override { currentState().m_pipelineStructure.state.graphics.primitiveTopology = topology; markPipelineDirty(); }
    void setPolygonMode(PolygonMode polygonMode) override { currentState().m_pipelineStructure.state.graphics.raster.polygonMode = polygonMode; markPipelineDirty(); }
    void setDepthCompareOp(CompareOp compareOp) override { currentState().m_pipelineStructure.state.graphics.depthStencil.depthCompareOp = compareOp; markPipelineDirty(); }
    void enableDepth(Bool enable) override { currentState().m_pipelineStructure.state.graphics.depthStencil.depthTestEnable = enable; markPipelineDirty(); }
    void enableStencil(Bool enable) override { currentState().m_pipelineStructure.state.graphics.depthStencil.stencilTestEnable = enable; markPipelineDirty(); }
    void setFrontFace(FrontFace frontFace) override { currentState().m_pipelineStructure.state.graphics.raster.frontFace = frontFace; markPipelineDirty(); }
    void setCullMode(CullMode cullMode) override { currentState().m_pipelineStructure.state.graphics.raster.cullMode = cullMode; markPipelineDirty(); }
    
    void setShaderProgram(ShaderProgramId program, U32 permutation) override 
    { 
        currentState().m_pipelineStructure.state.shaderProgramId = program; 
        currentState().m_pipelineStructure.state.shaderPermutation = permutation; 
    }

    void setInputVertexLayout(VertexInputLayoutId inputLayoutId) override { currentState().m_pipelineStructure.state.graphics.ia = inputLayoutId; markPipelineDirty(); }

    inline void endRenderPass(VkCommandBuffer buffer);
    void resetBinds();

    ShaderStageFlags obtainResourceViewShaderFlags(ResourceViewId id)
    {
        return m_resourceViewShaderAccessMap[id];
    }

    ShaderStageFlags obtainConstantBufferShaderFlags(ResourceId id)
    {
        return m_constantBufferShaderAccessMap[id];
    }

    ShaderStageFlags obtainSamplerShaderFlags(SamplerId id)
    {
        return m_samplerShaderAccessMap[id];
    }

    VulkanDevice* getNativeDevice() { return getDevice()->castTo<VulkanDevice>(); }

    void pushState(ContextFlags flags) override;
    void popState() override;

private:
    struct ContextState
    {
        Pipelines::Structure                                            m_pipelineStructure;
        DescriptorSets::Structure                                       m_boundDescriptorSetStructure;
        std::vector<VulkanResourceView*>                                m_srvs;
        std::vector<VulkanResourceView*>                                m_uavs;
        std::vector<VulkanBuffer*>                                      m_cbvs;
        std::vector<VulkanSampler*>                                     m_samplers;
    };

    void prepare();
    void initialize();
    void destroyFences();
    void createFences(U32 buffered);
    void bindPipelineState(const VulkanDescriptorAllocation& set);
    // Flushes barrier transitions if we need to. This must be called on any resources that will be accessed by 
// specific api calls that require the state of the resource to be the desired target at the time.
    void flushBarrierTransitions(VkCommandBuffer cmdBuffer);
    void setRenderPass(VulkanRenderPass* pPass);
    void bindDescriptorSet(const VulkanDescriptorAllocation& set);
    void markPipelineDirty() { m_pipelineDirty = true; }
    Bool isPipelineDirty() const { return m_pipelineDirty; }
    void unmarkPipelineDirty() { m_pipelineDirty = false; }
    ContextState& currentState() { return *m_contextStates.end(); }

    // buffer count 
    U32                                                                 m_bufferCount;
    U32                                                                 m_currentBufferIndex;
    ::std::vector<VkFence>                                              m_fences;
    VulkanDevice*                                                       m_pDevice;
    ::std::vector<VkBufferMemoryBarrier>                                m_bufferMemoryBarriers;
    ::std::vector<VkImageMemoryBarrier>                                 m_imageMemoryBarriers;
    VulkanRenderPass*                                                   m_boundRenderPass;
    VulkanPrimaryCommandList                                            m_primaryCommandList;
    Pipelines::PipelineState                                            m_pipelineState;
    PipelineId                                                          m_pipelineId;
    std::unordered_map<ResourceViewId, ShaderStageFlags>                m_resourceViewShaderAccessMap;
    std::unordered_map<ResourceId, ShaderStageFlags>                    m_constantBufferShaderAccessMap;
    std::unordered_map<SamplerId, ShaderStageFlags>                     m_samplerShaderAccessMap;
    std::vector<ContextState>                                           m_contextStates;
    VkDescriptorSet                                                     m_boundDescriptorSet;
    Bool                                                                m_pipelineDirty;
    U32                                                                 m_currentStateIdx;
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

    ErrType     destroySwapchain(VulkanSwapchain* pSwapchain);
    ErrType     createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc, ResourceState initState) override;
    ErrType     createSampler(GraphicsSampler** ppSampler, const SamplerCreateDesc& desc) override;
    ErrType     destroySampler(GraphicsSampler* pSampler) override;
    ErrType     destroyResource(GraphicsResource* pResource) override;
    ErrType     createResourceView(GraphicsResourceView** ppView, const ResourceViewDescription& desc) override;
    ErrType     loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition) override;
    ErrType     unloadShaderProgram(ShaderProgramId program) override;
    void        unloadAllShaderPrograms() override;
    Bool        makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) override;
    Bool        destroyVertexLayout(VertexInputLayoutId id) override;
    void        release(VkInstance instance);
    ErrType     destroyResourceView(GraphicsResourceView* pResourceView) override;

    VkDevice operator()() {
        return m_device;
    }

    VkDevice get() const {
        return m_device;
    }

    VkSurfaceKHR getSurface() const { return m_surface; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    VulkanAdapter* getAdapter() const { return m_adapter; }

    VulkanAllocationManager* getAllocationManager()
        { return m_allocationManager.raw(); }

    VulkanDescriptorManager* getDescriptorHeap() { return m_pDescriptorManager; }

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

    SmartPtr<VulkanAllocationManager> m_allocationManager;
    VulkanDescriptorManager*          m_pDescriptorManager;
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