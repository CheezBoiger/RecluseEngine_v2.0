// 
#pragma once

#include "VulkanInstance.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanShaderCache.hpp"
#include "VulkanResource.hpp"
#include "VulkanObjects.hpp"
#include "VulkanViews.hpp"
#include "VulkanQueue.hpp"
#include "VulkanDescriptorManager.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "VulkanCommandList.hpp"
#include "Recluse/Threading/Threading.hpp"

#include "RecluseVulkan_exports.hpp"

#include <vector>
#include <list>
#include <memory>
#include <array>

namespace Recluse {
class Allocator;
class MemoryPool;
struct DeviceCreateInfo;
} // Recluse

namespace Recluse {
namespace Vulkan {

class VulkanAdapter;
class VulkanQueue;
class VulkanSwapchain;
class VulkanDevice;
class VulkanSwapchain;


struct QueueFamily 
{
    U32                                     maxQueueCount;
    U32                                     queueFamilyIndex;
    U32                                     currentAvailableQueueIndex;
    VkQueueFlags                            flags;
};

enum ContextFrameFlag
{
    ContextFrameFlag_None = 0,
    ContextFrameFlag_SwapchainQueued = (1 << 0),
};

typedef U32 ContextFrameFlags;

// Vulkan Context frame. The gpu could potentially have more than one frame inflight, any available 
// frame that can be worked on by the host cpu, will be signaled by the fence value, and allocated command buffer.
struct VulkanContextFrame
{   
    VkSemaphore                 waitSemaphore;
    VkSemaphore                 signalSemaphore;
    VkFence                     fence;
    VulkanQueryManager          timestampQuery;
    VulkanQueryManager          occlusionQuery;
    BufferTemporaryAllocator    temporaryBufferAllocator;
    ContextFrameFlags           flags;
};


// Vulkan Context.
class RecluseVulkan_PUBLIC_API VulkanContext : public GraphicsContext
{
private:
    struct ContextState;
    class VulkanShaderProgramBinder : public IShaderProgramBinder
    {
    public:
        VulkanShaderProgramBinder(VulkanContext* context = nullptr, ShaderProgramId programId = ~0, ShaderPermutationId permutationId = ~0)
            : IShaderProgramBinder(programId, permutationId)
            , m_pContext(context) 
            , cachedProgram(nullptr)
            , reflectionCache(nullptr)
        { obtainShaderProgramFromCache(); }
        IShaderProgramBinder&   bindShaderResource(ShaderStageFlags type, U32 space, U32 slot, ResourceView view) override;
        IShaderProgramBinder&   bindUnorderedAccessView(ShaderStageFlags type, U32 space, U32 slot, ResourceView view) override;
        IShaderProgramBinder&   bindConstantBuffer(ShaderStageFlags type, U32 space, U32 slot, GraphicsResource* pResource, U32 offsetBytes, U32 sizeBytes, void* data = nullptr) override;
        IShaderProgramBinder&   bindConstantBuffer(ShaderStageFlags type, U32 space, U32 slot, ResourceView view) override;
        IShaderProgramBinder&   bindSampler(ShaderStageFlags type, U32 space, U32 slot, GraphicsSampler* ppSampler) override;
        ContextState&           currentState();
        ShaderProgramReflection* getReflection() const { return reflectionCache; }
    private:
        void                    obtainShaderProgramFromCache();
        VulkanContext* m_pContext;
        ShaderPrograms::VulkanShaderProgram*    cachedProgram;
        ShaderProgramReflection*                reflectionCache;
    };

public:
    VulkanContext(VulkanDevice* pDevice, VulkanQueue* queue, VulkanQueue* computeQueue = nullptr)
        : m_bufferCount(0)
        , m_currentContextFrameIndex(0)
        , m_boundRenderPass(nullptr)
        , m_pDevice(pDevice) 
        , m_graphicsQueue(queue)
        , m_computeQueue(computeQueue)
    { 
    }

    ~VulkanContext();
    
    void                            release();
    void                            begin() override;
    void                            end() override;

    inline U32                      getFrameCount() const { return m_bufferCount; }
    inline U32                      getCurrentFrameIndex() const { return m_currentContextFrameIndex; }
    inline VkFence                  getCurrentFence() const { return m_frameResources[m_currentContextFrameIndex].fence; }
    ResultCode                      createPrimaryCommandList(VkQueueFlags flags);
    ResultCode                      destroyPrimaryCommandList();
    ResultCode                      setFrames(U32 newBufferCount) override;
    U32                             obtainCurrentFrameIndex() const override { return getCurrentFrameIndex(); }
    U32                             obtainFrameCount() const override { return getFrameCount(); }
    DescriptorAllocatorInstance*    currentDescriptorAllocator();

    ResourceView                    allocateConstantBuffer(U32 cbSizeBytes, void* dat) override;

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
    Bool                            supportsAsyncCompute() const override;

    void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) override;
    void clearDepthStencil(ClearFlags flags, F32 clearDepth, U8 clearStencil, const Rect& rect) override;

    void drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) override;
    void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) override;
    void bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) override;
    void drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance) override;

    void drawInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount) override;
    void drawIndexedInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount) override;
    void dispatchIndirect(GraphicsResource* pParams, U64 offset) override;

    void dispatchMesh(U32 x, U32 y, U32 z) override;
    void dispatchMeshIndirect(GraphicsResource* indirectBuffer, U32 offset, U32 drawCount) override;

    void setViewports(U32 numViewports, Viewport* pViewports) override;
    void setScissors(U32 numScissors, Rect* pRects) override;
    void dispatch(U32 x, U32 y, U32 z) override;

    void beginLabel(const char* label, const Math::Float4& color) override;
    void endLabel() override;

    GraphicsQuery beginQuery(GraphicsQueryType type) override;
    void endQuery(const GraphicsQuery& query) override;

    void transition(GraphicsResource* pResource, ResourceState dstState, U16 baseMip, U16 mipCount, U16 baseLayer, U16 layerCount) override;

    void bindBlendState(const BlendState& state) override { currentState().m_pipelineStructure.state.pipeline.graphics.blendState = state; currentState().markPipelineDirty(); }
    void setTopology(PrimitiveTopology topology) override { currentState().m_pipelineStructure.state.pipeline.graphics.primitiveTopology = topology; currentState().markPipelineDirty(); }
    void setPolygonMode(PolygonMode polygonMode) override { currentState().m_pipelineStructure.state.pipeline.graphics.raster.polygonMode = polygonMode; currentState().markPipelineDirty(); }
    void setDepthCompareOp(CompareOp compareOp) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.depthCompareOp = compareOp; currentState().markPipelineDirty(); }
    void enableDepth(Bool enable) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.depthTestEnable = enable; currentState().markPipelineDirty(); }
    void enableStencil(Bool enable) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.stencilTestEnable = enable; currentState().markPipelineDirty(); }
    void setFrontFace(FrontFace frontFace) override { currentState().m_pipelineStructure.state.pipeline.graphics.raster.frontFace = frontFace; currentState().markPipelineDirty(); }
    void setCullMode(CullMode cullMode) override { currentState().m_pipelineStructure.state.pipeline.graphics.raster.cullMode = cullMode; currentState().markPipelineDirty(); }
    void setLineWidth(F32 width) override { currentState().m_pipelineStructure.state.pipeline.graphics.raster.lineWidth = width; currentState().markPipelineDirty(); }
    void setBlendEnable(U32 rtIndex, Bool enable) override { currentState().m_pipelineStructure.state.pipeline.graphics.blendState.attachments[rtIndex].blendEnable = enable; currentState().markPipelineDirty(); }
    void setBlendLogicOpEnable(Bool enable) override { currentState().m_pipelineStructure.state.pipeline.graphics.blendState.logicOpEnable = enable; }
    void setBlendLogicOp(LogicOp logicOp) override { currentState().m_pipelineStructure.state.pipeline.graphics.blendState.logicOp = logicOp; }
    void enableDepthWrite(Bool enable) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.depthWriteEnable = enable; currentState().markPipelineDirty(); }
    void setStencilReference(U8 ref) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.stencilReference = ref; currentState().markPipelineDirty(); }
    void setStencilReadMask(U8 mask) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.stencilReadMask = mask; currentState().markPipelineDirty(); }
    void setStencilWriteMask(U8 mask) override { currentState().m_pipelineStructure.state.pipeline.graphics.depthStencil.stencilWriteMask = mask; currentState().markPipelineDirty(); }
    void clearResourceBinds() override;
    void bindRenderTargets(U32 count, ResourceView* ppResources, ResourceView pDepthStencil) override;
    
    void setBlendConstants(F32 blendConstants[4]) override 
    { 
        F32* blendStateConstants = currentState().m_pipelineStructure.state.pipeline.graphics.blendState.blendConstants;
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
        BlendState& blendState = currentState().m_pipelineStructure.state.pipeline.graphics.blendState;
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
        BlendState& blendState = currentState().m_pipelineStructure.state.pipeline.graphics.blendState;
        blendState.attachments[rtIndex].colorWriteMask      = writeMask;
        currentState().markPipelineDirty();
    }
    
    IShaderProgramBinder& bindShaderProgram(ShaderProgramId program, U32 permutation) override 
    { 
        currentState().m_pipelineStructure.state.shaderProgramId = program; 
        currentState().m_pipelineStructure.state.shaderPermutation = permutation; 
        currentState().markPipelineDirty();

        // Binds and asserts any possible missing slots, if there is reflection data. Otherwise,
        // will act as a normal binder without knowledge of the program.
        m_shaderProgramBinder = VulkanShaderProgramBinder(this, program, permutation);
        return m_shaderProgramBinder;
    }

    void setInputVertexLayout(VertexInputLayoutId inputLayoutId) override 
    { 
        currentState().m_pipelineStructure.state.pipeline.graphics.ia = inputLayoutId; 
        currentState().markPipelineDirty(); 
    }

    void internalBindVertexBuffersAndIndexBuffer();

    void endRenderPass(VkCommandBuffer buffer);
    void resetBinds();

    ShaderStageFlags obtainResourceViewShaderFlags(ResourceView id)
    {
        return m_resourceViewShaderAccessMap[id.ptr];
    }

    ShaderStageFlags obtainConstantBufferShaderFlags(VkBuffer buffer)
    {
        return m_constantBufferShaderAccessMap[buffer];
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
    VulkanContextFrame& getContextFrame(U32 idx) { return m_frameResources[idx]; }

private:
    enum ContextDirtyFlag
    {
        ContextDirtyFlag_Clean          = (0),
        ContextDirtyFlag_Resources      = (1 << 0),
        ContextDirtyFlag_Pipeline       = (1 << 1),
        ContextDirtyFlag_VertexBuffers  = (1 << 2),
        ContextDirtyFlag_IndexBuffer    = (1 << 3)
    };

    typedef U32 ContextDirtyFlags;

    struct ContextState
    {
        Pipelines::Structure                                                    m_pipelineStructure;
        std::vector<DescriptorSets::Structure>                                  m_boundDescriptorSetStructures;
        // Need to set this on their own sets.
        std::vector<DescriptorSets::VulkanSet>                                  m_boundPerSet;
        std::array<VkBuffer, 16>                                                m_vertexBuffers;
        std::array<U64, 16>                                                     m_vbOffsets;
        VkBuffer                                                                m_indexBuffer;
        ContextDirtyFlags                                                       m_dirtyFlags;
        U8                                                                      m_numBoundVBs;
        VkIndexType                                                             m_ibType;
        VkDeviceSize                                                            m_ibOffsetBytes;

        void setDirty(ContextDirtyFlags flags) { m_dirtyFlags |= flags; }
        void markPipelineDirty() { m_dirtyFlags |= ContextDirtyFlag_Pipeline; }
        void markResourcesDirty() { m_dirtyFlags |= ContextDirtyFlag_Resources; };
        Bool isPipelineDirty() const { return (m_dirtyFlags & ContextDirtyFlag_Pipeline); }
        Bool areResourcesDirty() const { return (m_dirtyFlags & ContextDirtyFlag_Resources); }
        Bool areVertexBuffersOrIndexBuffersDirty() const { return m_dirtyFlags & (ContextDirtyFlag_IndexBuffer | ContextDirtyFlag_VertexBuffers); }
        void proposeClean() { m_dirtyFlags = ContextDirtyFlag_Clean; }
    };

    void prepare();
    void initialize(U32 bufferCount);
    void destroyContextFrames();
    void createContextFrames(U32 buffered);
    void bindPipelineState(const VulkanDescriptorAllocation& set);
    // Flushes barrier transitions if we need to. This must be called on any resources that will be accessed by 
// specific api calls that require the state of the resource to be the desired target at the time.
    void flushBarrierTransitions(VkCommandBuffer cmdBuffer);
    void setRenderPass(const VulkanRenderPass& pPass);
    void bindDescriptorSet(const VulkanDescriptorAllocation& set);

    // Submit the final command buffer, which will utilize the wait and signal semphores for the current frame.
    // Can optionally override the frame inflight fence with your own fence if needed. Keep in mind this can be 
    // dangerous!
    ResultCode                  submitFinalCommandBuffer(VkCommandBuffer commandBuffer);
    
    inline void incrementContextFrameIndex() 
    { 
        m_currentContextFrameIndex = (m_currentContextFrameIndex + 1) % m_bufferCount; 
    }

    // The current context state, that is pushed to this context.
    ContextState& currentState() { return m_contextStates.back(); }

    ResultCode createCommandPools(U32 buffered);
    void destroyCommandPools();
    void resetCommandPool(U32 bufferIdx, Bool resetAllResources);

    // buffer count 
    U32                                                                 m_bufferCount;
    U32                                                                 m_currentContextFrameIndex;
    ::std::vector<VulkanContextFrame>                                   m_frameResources;
    VulkanDevice*                                                       m_pDevice;
    ::std::vector<VkBufferMemoryBarrier>                                m_bufferMemoryBarriers;
    ::std::vector<VkImageMemoryBarrier>                                 m_imageMemoryBarriers;
    VulkanRenderPass                                                    m_newRenderPass;
    VkRenderPass                                                        m_boundRenderPass;
    VulkanPrimaryCommandList                                            m_primaryCommandList;
    Pipelines::PipelineState                                            m_pipelineState;
    PipelineId                                                          m_pipelineId;
    std::unordered_map<ResourceId, ShaderStageFlags>                    m_resourceViewShaderAccessMap;
    std::unordered_map<VkBuffer, ShaderStageFlags>                      m_constantBufferShaderAccessMap;
    std::unordered_map<SamplerId, ShaderStageFlags>                     m_samplerShaderAccessMap;
    std::vector<VkCommandPool>                                          m_commandPools;
    std::vector<ContextState>                                           m_contextStates;
    VkDescriptorSet                                                     m_boundDescriptorSet;
    U32                                                                 m_currentStateIdx;
    VulkanQueue*                                                        m_graphicsQueue;
    VulkanQueue*                                                        m_computeQueue;
    VulkanShaderProgramBinder                                           m_shaderProgramBinder;
};


class RecluseVulkan_PUBLIC_API VulkanDevice : public GraphicsDevice 
{
public:
    VulkanDevice()
        : m_device(VK_NULL_HANDLE)
        , m_adapter(nullptr)
        , m_enabledFeatures({ })
        , m_memCache({ })
        , m_supportsSwapchainCreation(false)
    { 
    }

    ResultCode              initialize(VulkanAdapter* iadapter, DeviceCreateInfo& info, U32 deviceId);
    ResultCode              createQueues();
    const std::vector<QueueFamily>& getQueueFamilies() const { return m_queueFamilies; }    


    VulkanQueue             makeQueue(VkQueueFlags flags, Bool reuse = false, VkSurfaceKHR surfaceToPresent = VK_NULL_HANDLE);
    ResultCode              destroyQueues();
    
    GraphicsSwapchain*      createSwapchain(const SwapchainCreateDescription& description, void* windowHandle) override;
    ResultCode              destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    GraphicsContext*        createContext() override;
    ResultCode              releaseContext(GraphicsContext* pContext) override;

    ResultCode              destroySwapchain(VulkanSwapchain* pSwapchain);
    ResultCode              createResource(GraphicsResource** ppResource, const GraphicsResourceDescription& pDesc, ResourceState initState) override;
    ResultCode              createSampler(GraphicsSampler** ppSampler, const SamplerDescription& desc) override;
    ResultCode              destroySampler(GraphicsSampler* pSampler) override;
    ResultCode              destroyResource(GraphicsResource* pResource, Bool immediate) override;
    ResultCode              loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition) override;
    ResultCode              unloadShaderProgram(ShaderProgramId program) override;
    void                    unloadAllShaderPrograms() override;
    Bool                    makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) override;
    Bool                    destroyVertexLayout(VertexInputLayoutId id) override;
    void                    release(VkInstance instance);
    void                    copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, const CopyBufferRegion* regions, U32 numRegions) override;
    void                    copyResource(GraphicsResource* dst, GraphicsResource* src) override;
    VulkanQueue*            getQueue(VkQueueFlags flags) { auto& iter = m_queues.find(flags); if (iter != m_queues.end()) return &iter->second; return nullptr; }
    VulkanQueue*            getPresentableQueue(VkSurfaceKHR surface);
    VkMemoryRequirements    getBufferMemoryRequirements(VkBuffer buffer) const;
    VkMemoryRequirements    getImageMemoryRequirements(VkImage image) const;

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

    ResultCode                      reserveMemory(const MemoryReserveDescription& desc) override;
    VulkanAdapter*                  getAdapter() const { return m_adapter; }

    VulkanAllocationManager*        getAllocationManager()
        { return m_allocationManager.raw(); }

    // Invalidate resources when reading back from GPU. This invalidates memory ranges so that the host (CPU),
    // will be able to see GPU writes to the memory.
    void                            pushInvalidateMemoryRange(const VkMappedMemoryRange& mappedRange);

    // Flush the resource memory range when writing from CPU to GPU. This flushes memory ranges so that the device (GPU)
    // will be able to see CPU writes to the memory.
    void                            pushFlushMemoryRange(const VkMappedMemoryRange& mappedRange);

    void                            flushAllMappedRanges();
    void                            invalidateAllMappedRanges();
    VkDeviceSize                    getNonCoherentSize() const;

    const VkPhysicalDeviceFeatures& getEnabledFeatures() const { return m_enabledFeatures; }

private:

    void                            loadFunctions();

    PFN_vkVoidFunction              getProcAddr(const char* procName);
    void                            allocateMemCache();
    void                            createDescriptorHeap();
    void                            destroyDescriptorHeap();
    void                            freeMemCache();

    VulkanAdapter*                  m_adapter;
    VkDevice                        m_device;

    struct MemoryManager
    {
        std::unique_ptr<MemoryPool> pool;
        std::unique_ptr<Allocator> allocator;
        CriticalSection             cs;

        void initialize();
        void free();
    };

    struct 
    {
        MemoryManager invalid;
        MemoryManager flush;
    } m_memCache;

    SmartPtr<VulkanAllocationManager>   m_allocationManager;
    std::vector<VulkanContext*>         m_allocatedContexts;
    std::vector<QueueFamily>            m_queueFamilies;
    DescriptorAllocator                 m_descriptorAllocator;

    // Cache the enabled features that are available for this device.
    VkPhysicalDeviceFeatures            m_enabledFeatures;
    std::map<VkQueueFlags, VulkanQueue> m_queues;
    Bool                                m_supportsSwapchainCreation;
};
} // Vulkan
} // Recluse