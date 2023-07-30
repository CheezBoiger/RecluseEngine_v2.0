// 
#pragma once

#include "VulkanInstance.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanShaderCache.hpp"
#include "VulkanResource.hpp"
#include "VulkanObjects.hpp"
#include "VulkanViews.hpp"
#include "VulkanDescriptorManager.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "VulkanCommandList.hpp"
#include <vector>
#include <list>
#include <array>

namespace Recluse {


class VulkanAdapter;
class VulkanQueue;
class VulkanSwapchain;
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
};


class VulkanContext : public GraphicsContext
{
public:
    VulkanContext(VulkanDevice* pDevice)
        : m_bufferCount(0)
        , m_currentBufferIndex(0)
        , m_boundRenderPass(nullptr)
        , m_newRenderPass(nullptr)
        , m_pDevice(pDevice) 
    { 
    }

    ~VulkanContext();
    
    void                            release();
    void                            begin() override;
    void                            end() override;

    inline U32                      getBufferCount() const { return m_bufferCount; }
    inline U32                      getCurrentBufferIndex() const { return m_currentBufferIndex; }
    inline VkFence                  getCurrentFence() const { return m_fences[m_currentBufferIndex]; }
    ResultCode                      createPrimaryCommandList(VkQueueFlags flags);
    ResultCode                      destroyPrimaryCommandList();
    ResultCode                      setBuffers(U32 newBufferCount);
    U32                             obtainCurrentBufferIndex() const { return getCurrentBufferIndex(); }
    U32                             obtainBufferCount() const { return getBufferCount(); }
    DescriptorAllocatorInstance*    currentDescriptorAllocator();

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    void                            copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    void                            copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
                                        const CopyBufferRegion* pRegions, U32 numRegions) override;

    GraphicsDevice*                 getDevice() override;
    VulkanPrimaryCommandList*       getPrimaryCommandList() { return &m_primaryCommandList; }
    ResultCode                      wait() override;
    VkRenderPass                    getRenderPass();
    Bool                            supportsAsyncCompute() const;

    void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) override;
    void clearDepthStencil(ClearFlags flags, F32 clearDepth, U8 clearStencil, const Rect& rect) override;

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

    void bindShaderResources(ShaderType type, U32 offset, U32 count, ResourceViewId* ppResources) override;
    void bindUnorderedAccessViews(ShaderType type, U32 offset, U32 count, ResourceViewId* ppResources) override;
    void bindConstantBuffer(ShaderType type, U32 slot, GraphicsResource* pResource, U64 offsetBytes, U64 sizeBytes) override;
    void bindRenderTargets(U32 count, ResourceViewId* ppResources, ResourceViewId pDepthStencil) override;
    void bindSamplers(ShaderType type, U32 count, GraphicsSampler** ppSampler) override;
    void bindRasterizerState(const RasterState& state) override { }
    void bindBlendState(const BlendState& state) override { currentState().m_pipelineStructure.state.graphics.blendState = state; currentState().markPipelineDirty(); }
    void setTopology(PrimitiveTopology topology) override { currentState().m_pipelineStructure.state.graphics.primitiveTopology = topology; currentState().markPipelineDirty(); }
    void setPolygonMode(PolygonMode polygonMode) override { currentState().m_pipelineStructure.state.graphics.raster.polygonMode = polygonMode; currentState().markPipelineDirty(); }
    void setDepthCompareOp(CompareOp compareOp) override { currentState().m_pipelineStructure.state.graphics.depthStencil.depthCompareOp = compareOp; currentState().markPipelineDirty(); }
    void enableDepth(Bool enable) override { currentState().m_pipelineStructure.state.graphics.depthStencil.depthTestEnable = enable; currentState().markPipelineDirty(); }
    void enableStencil(Bool enable) override { currentState().m_pipelineStructure.state.graphics.depthStencil.stencilTestEnable = enable; currentState().markPipelineDirty(); }
    void setFrontFace(FrontFace frontFace) override { currentState().m_pipelineStructure.state.graphics.raster.frontFace = frontFace; currentState().markPipelineDirty(); }
    void setCullMode(CullMode cullMode) override { currentState().m_pipelineStructure.state.graphics.raster.cullMode = cullMode; currentState().markPipelineDirty(); }
    void setLineWidth(F32 width) override { currentState().m_pipelineStructure.state.graphics.raster.lineWidth = width; currentState().markPipelineDirty(); }
    void setBlendEnable(U32 rtIndex, Bool enable) override { currentState().m_pipelineStructure.state.graphics.blendState.attachments[rtIndex].blendEnable = enable; currentState().markPipelineDirty(); }
    void setBlendLogicOpEnable(Bool enable) override { currentState().m_pipelineStructure.state.graphics.blendState.logicOpEnable = enable; }
    void setBlendLogicOp(LogicOp logicOp) override { currentState().m_pipelineStructure.state.graphics.blendState.logicOp = logicOp; }
    
    void setBlendConstants(F32 blendConstants[4]) override 
    { 
        F32* blendStateConstants = currentState().m_pipelineStructure.state.graphics.blendState.blendConstants;
        blendStateConstants[0] = blendConstants[0]; 
        blendStateConstants[1] = blendConstants[1];
        blendStateConstants[2] = blendConstants[2];
        blendStateConstants[3] = blendConstants[3];
        currentState().markPipelineDirty();
    }
    
    void setBlend
                (
                    U32 rtIndex, 
                    BlendFactor srcColorFactor, BlendFactor dstColorFactor, BlendOp colorBlendOp,
                    BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor, BlendOp alphaOp
                ) override
    {
        BlendState& blendState = currentState().m_pipelineStructure.state.graphics.blendState;
        blendState.attachments[rtIndex].srcColorBlendFactor = srcColorFactor;
        blendState.attachments[rtIndex].dstColorBlendFactor = dstColorFactor;
        blendState.attachments[rtIndex].colorBlendOp        = colorBlendOp;
        blendState.attachments[rtIndex].srcAlphaBlendFactor = srcAlphaFactor;
        blendState.attachments[rtIndex].dstAlphaBlendFactor = dstAlphaFactor;
        blendState.attachments[rtIndex].alphaBlendOp        = alphaOp;
        currentState().markPipelineDirty();
    }
                                
    void setColorWriteMask(U32 rtIndex, ColorComponentMaskFlags writeMask) override
    {
        BlendState& blendState = currentState().m_pipelineStructure.state.graphics.blendState;
        blendState.attachments[rtIndex].colorWriteMask      = writeMask;
        currentState().markPipelineDirty();
    }
    
    void setShaderProgram(ShaderProgramId program, U32 permutation) override 
    { 
        currentState().m_pipelineStructure.state.shaderProgramId = program; 
        currentState().m_pipelineStructure.state.shaderPermutation = permutation; 
        currentState().markPipelineDirty();
    }

    void setInputVertexLayout(VertexInputLayoutId inputLayoutId) override 
    { 
        currentState().m_pipelineStructure.state.graphics.ia = inputLayoutId; 
        currentState().markPipelineDirty(); 
    }

    void endRenderPass(VkCommandBuffer buffer);
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

    VkCommandPool* getCommandPools() { return m_commandPools.data(); }
    U32 getNumCommandPools() const { return static_cast<U32>(m_commandPools.size()); }

private:
    enum ContextDirtyFlag
    {
        ContextDirtyFlag_Clean = (0),
        ContextDirtyFlag_Resources = (1 << 0),
        ContextDirtyFlag_Pipeline = (1 << 1)
    };

    typedef U32 ContextDirtyFlags;

    struct ContextState
    {
        Pipelines::Structure                                            m_pipelineStructure;
        DescriptorSets::Structure                                       m_boundDescriptorSetStructure;
        std::vector<VulkanResourceView*>                                m_srvs;
        std::vector<VulkanResourceView*>                                m_uavs;
        std::array<DescriptorSets::ConstantBuffer, 16>                  m_cbvs;
        std::vector<VulkanSampler*>                                     m_samplers;
        ContextDirtyFlags                                               m_dirtyFlags;

        void setDirty(ContextDirtyFlags flags) { m_dirtyFlags = flags; }
        void markPipelineDirty() { m_dirtyFlags |= ContextDirtyFlag_Pipeline; }
        void markResourcesDirty() { m_dirtyFlags |= ContextDirtyFlag_Resources; };
        Bool isPipelineDirty() const { return (m_dirtyFlags & ContextDirtyFlag_Pipeline); }
        Bool areResourcesDirty() const { return (m_dirtyFlags & ContextDirtyFlag_Resources); }
        void proposeClean() { m_dirtyFlags = ContextDirtyFlag_Clean; }
    };

    void prepare();
    void initialize(U32 bufferCount);
    void destroyFences();
    void createFences(U32 buffered);
    void bindPipelineState(const VulkanDescriptorAllocation& set);
    // Flushes barrier transitions if we need to. This must be called on any resources that will be accessed by 
// specific api calls that require the state of the resource to be the desired target at the time.
    void flushBarrierTransitions(VkCommandBuffer cmdBuffer);
    void setRenderPass(VulkanRenderPass* pPass);
    void bindDescriptorSet(const VulkanDescriptorAllocation& set);
    
    inline void incrementBufferIndex() 
    { 
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount; 
    }

    // The current context state, that is pushed to this context.
    ContextState& currentState() { return m_contextStates.back(); }

    ResultCode createCommandPools(U32 buffered);
    void destroyCommandPools();
    void resetCommandPool(U32 bufferIdx, Bool resetAllResources);

    // buffer count 
    U32                                                                 m_bufferCount;
    U32                                                                 m_currentBufferIndex;
    ::std::vector<VkFence>                                              m_fences;
    VulkanDevice*                                                       m_pDevice;
    ::std::vector<VkBufferMemoryBarrier>                                m_bufferMemoryBarriers;
    ::std::vector<VkImageMemoryBarrier>                                 m_imageMemoryBarriers;
    VulkanRenderPass*                                                   m_newRenderPass;
    VkRenderPass                                                        m_boundRenderPass;
    VulkanPrimaryCommandList                                            m_primaryCommandList;
    Pipelines::PipelineState                                            m_pipelineState;
    PipelineId                                                          m_pipelineId;
    std::unordered_map<ResourceViewId, ShaderStageFlags>                m_resourceViewShaderAccessMap;
    std::unordered_map<ResourceId, ShaderStageFlags>                    m_constantBufferShaderAccessMap;
    std::unordered_map<SamplerId, ShaderStageFlags>                     m_samplerShaderAccessMap;
    std::vector<VkCommandPool>                                          m_commandPools;
    std::vector<ContextState>                                           m_contextStates;
    VkDescriptorSet                                                     m_boundDescriptorSet;
    U32                                                                 m_currentStateIdx;
    U32                                                                 m_queueFamilyIndex;
};


class VulkanDevice : public GraphicsDevice 
{
public:
    VulkanDevice()
        : m_device(VK_NULL_HANDLE)
        , m_surface(VK_NULL_HANDLE)
        , m_windowHandle(nullptr)
        , m_adapter(nullptr)
        , m_pGraphicsQueue(nullptr)
        , m_pAsyncComputeQueue(nullptr)
        , m_enabledFeatures({ })
        , m_memCache({ })
        , m_swapchain(nullptr)
    { 
    }

    ResultCode          initialize(VulkanAdapter* iadapter, DeviceCreateInfo& info);
    VulkanQueue*        getBackbufferQueue() { return m_pGraphicsQueue; }
    VulkanQueue*        getAsyncQueue() { return m_pAsyncComputeQueue; }
    ResultCode          createQueues();
    const std::vector<QueueFamily>& getQueueFamilies() const { return m_queueFamilies; }    


    ResultCode          createQueue(VulkanQueue** ppQueue, VkQueueFlags flags, B32 isPresentable);
    ResultCode          destroyQueues();

    ResultCode          createSwapchain(VulkanSwapchain** ppSwapchain, 
                            const SwapchainCreateDescription& pDesc, VulkanQueue* pPresentationQueue);

    GraphicsContext*    createContext() override;
    ResultCode          releaseContext(GraphicsContext* pContext) override;

    ResultCode          destroySwapchain(VulkanSwapchain* pSwapchain);
    ResultCode          createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc, ResourceState initState) override;
    ResultCode          createSampler(GraphicsSampler** ppSampler, const SamplerDescription& desc) override;
    ResultCode          destroySampler(GraphicsSampler* pSampler) override;
    ResultCode          destroyResource(GraphicsResource* pResource) override;
    ResultCode          loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition) override;
    ResultCode          unloadShaderProgram(ShaderProgramId program) override;
    void                unloadAllShaderPrograms() override;
    Bool                makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) override;
    Bool                destroyVertexLayout(VertexInputLayoutId id) override;
    void                release(VkInstance instance);
    void                copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, const CopyBufferRegion* regions, U32 numRegions) override;

    DescriptorAllocatorInstance*    getDescriptorAllocatorInstance(U32 bufferIndex)
    {
        return m_descriptorAllocator.getInstance(bufferIndex);
    }

    DescriptorAllocator*            getDescriptorAllocator() { return &m_descriptorAllocator; }

    VkDevice operator()() 
    {
        return m_device;
    }

    VkDevice get() const 
    {
        return m_device;
    }

    VkSurfaceKHR                    getSurface() const { return m_surface; }
    ResultCode                      reserveMemory(const MemoryReserveDescription& desc) override;
    VulkanAdapter*                  getAdapter() const { return m_adapter; }

    VulkanAllocationManager*        getAllocationManager()
        { return m_allocationManager.raw(); }

    // Invalidate resources when reading back from GPU.
    void                            pushInvalidateMemoryRange(const VkMappedMemoryRange& mappedRange);

    // Flush the resource memory range when writing from CPU to GPU.
    void                            pushFlushMemoryRange(const VkMappedMemoryRange& mappedRange);

    void                            flushAllMappedRanges();
    void                            invalidateAllMappedRanges();
    VkDeviceSize                    getNonCoherentSize() const;
    GraphicsSwapchain*              getSwapchain() override;
    Bool                            hasSurface() const { return m_surface != VK_NULL_HANDLE; }

    const VkPhysicalDeviceFeatures& getEnabledFeatures() const { return m_enabledFeatures; }

private:

    ResultCode                      createSurface(VkInstance instance, void* handle);
    void                            allocateMemCache();
    void                            createDescriptorHeap();
    void                            destroyDescriptorHeap();
    void                            freeMemCache();

    VulkanAdapter*                  m_adapter;
    VkDevice                        m_device;
    VkSurfaceKHR                    m_surface;
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

    SmartPtr<VulkanAllocationManager>   m_allocationManager;
    VulkanSwapchain*                    m_swapchain;
    std::vector<VulkanContext*>         m_allocatedContexts;
    std::vector<QueueFamily>            m_queueFamilies;
    VulkanQueue*                        m_pGraphicsQueue;
    VulkanQueue*                        m_pAsyncComputeQueue;
    DescriptorAllocator                 m_descriptorAllocator;
    
    void*                               m_windowHandle;

    // Cache the enabled features that are available for this device.
    VkPhysicalDeviceFeatures            m_enabledFeatures;
};
} // Recluse