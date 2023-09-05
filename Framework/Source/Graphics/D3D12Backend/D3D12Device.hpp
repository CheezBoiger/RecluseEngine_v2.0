//
#pragma once

#include "Win32/Win32Common.hpp"
#include "D3D12Commons.hpp"
#include "D3D12DescriptorTableManager.hpp"
#include "D3D12Allocator.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Queue.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "D3D12PipelineState.hpp"
#include <array>

namespace Recluse {

class D3D12Adapter;
class D3D12ResourcePagedAllocator;
class D3D12ResourceAllocationManager;
class D3D12Swapchain;
class D3D12Queue;
class D3D12PrimaryCommandList;
class D3D12RenderPass;
class D3D12Resource;
class D3D12Device;

struct ContextFrame 
{
    ID3D12CommandAllocator* pAllocator;
    U64                     fenceValue;
};


class D3D12Context : public GraphicsContext
{
public:
    D3D12Context(D3D12Device* pDevice, U32 bufferCount, D3D12Queue* pQueue)
        : m_pDevice(pDevice)
        , m_currentBufferIndex(0)
        , m_pPrimaryCommandList(nullptr)
        , m_bufferCount(bufferCount)
        , m_queue(pQueue)
    {

    }

    void                                initialize();
    void                                release();
    virtual void                        begin() override;
    virtual void                        end() override;
    ResultCode                          setBuffers(U32 bufferCount) override;
    ResultCode                          wait() override;

    inline void                         incrementBufferIndex() 
    { 
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount; 
    }

    U32                                 currentBufferIndex() const 
    { 
        return m_currentBufferIndex; 
    }

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    void                                copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    void                                copyBufferRegions
                                            (
                                                GraphicsResource* dst, 
                                                GraphicsResource* src, 
                                                const CopyBufferRegion* pRegions, 
                                                U32 numRegions
                                            ) override;

    ContextFrame*                    getCurrentContextFrame() { return &m_contextFrames[m_currentBufferIndex]; }
    U32                                 getCurrentBufferIndex() const { return m_currentBufferIndex; }
    U32                                 obtainCurrentBufferIndex() const { return getCurrentBufferIndex(); }
    U32                                 obtainBufferCount() const { return m_bufferCount; }

    const std::vector<ContextFrame>& getContextFrames() const { return m_contextFrames; }
    void                                resetCurrentResources();
   
    void                                bindRenderTargets(U32 count, ResourceViewId* ppResources, ResourceViewId pDepthStencil = 0) override;
    void                                clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) override;
    void                                clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect) override;
    void                                setScissors(U32 numScissors, Rect* pRects) override;
    void                                setViewports(U32 numViewports, Viewport* pViewports) override;

    void                                setShaderProgram(ShaderProgramId program, U32 permutation) override;
    void                                bindShaderResource(ShaderStageFlags type, U32 slot, ResourceViewId view) override;
    void                                bindUnorderedAccessView(ShaderStageFlags type, U32 slot, ResourceViewId view) override;
    void                                bindConstantBuffer(ShaderStageFlags type, U32 slot, GraphicsResource* pResource, U32 offsetBytes, U32 sizeBytes) override;
    void                                bindSampler(ShaderStageFlags type, U32 slot, GraphicsSampler* pSampler) override;
    void                                drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) override;
    void                                drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance) override;
    void                                bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) override;
    void                                setDepthCompareOp(CompareOp compareOp) override;
    void                                setDepthBiasEnable(Bool enable) override;
    void                                setDepthClampEnable(Bool enable) override;
    void                                enableDepthWrite(Bool enable) override;
    void                                enableDepth(Bool enable) override;
    void                                enableStencil(Bool enable) override;
    void                                clearResourceBinds() override;
    void                                setColorWriteMask(U32 rtIndex, ColorComponentMaskFlags writeMask) override;
    void                                setStencilReference(U8 stencilRef) override;
    void                                setStencilWriteMask(U8 mask) override;
    void                                setStencilReadMask(U8 mask) override;
    void                                setInputVertexLayout(VertexInputLayoutId inputLayout) override;
    void                                setTopology(PrimitiveTopology topology) override;
    void                                setFrontFace(FrontFace frontFace) override;
    void                                dispatch(U32 x, U32 y, U32 z) override;

    void                                bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) override;

    ID3D12GraphicsCommandList*          currentGraphicsCommandList() { return m_pPrimaryCommandList->get(); }
    void                                pushState(ContextFlags flags = ContextFlag_None) override;
    void                                popState() override;

    void                                transition(GraphicsResource* pResource, ResourceState newState, U16 baseMip, U16 mipCount, U16 baseLayer, U16 layerCount) override;
    GraphicsDevice*                     getDevice() override;
    const ContextFrame&              getContextFrame(U32 idx) const { return m_contextFrames[idx]; }
    void                                setNewFenceValue(U32 idx, U64 value) { m_contextFrames[idx].fenceValue = value; }   

private:
    typedef U32 ContextDirtyFlags;
    enum ContextDirty
    {
        ContextDirty_Clean      = 0,
        ContextDirty_Descriptors  = (1 << 0),
        ContextDirty_SamplerDescriptors = (1 << 1),
        ContextDirty_Pipeline = (1 << 2),
        ContextDirty_StencilRef = (1 << 3),
        ContextDirty_VertexBuffers = (1 << 4),
        ContextDirty_IndexBuffer = (1 << 5),
        ContextDirty_Topology = (1 << 6),
    };

    struct ContextState
    {
        Pipelines::PipelineStateObject                  m_pipelineStateObject;
        Pipelines::RootSigLayout                        m_rootSigLayout;
        Pipelines::RootSigResourceTable                 m_resourceTable;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 128>    m_srvs;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 8>      m_uavs;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 16>     m_cbvs;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 16>     m_samplers;
        std::array<D3D12_VERTEX_BUFFER_VIEW, 16>        m_vertexBuffers;
        PrimitiveTopology                               m_primitiveTopology;
        ContextDirtyFlags                               m_dirtyFlags;
        U32                                             m_numBoundVertexBuffers;

        void setDirty(ContextDirtyFlags flags) { m_dirtyFlags |= flags; }
        void setClean() { m_dirtyFlags = ContextDirty_Clean; }
        Bool isDirty(ContextDirtyFlags flagsToCheck) { return (m_dirtyFlags & flagsToCheck); }
    };

    void                                flushBarrierTransitions();
    void                                initializeBufferResources(U32 buffering);
    void                                destroyBufferResources();
    void                                prepare();
    ContextState&                       currentState() { return m_contextStates.back(); }
    ResultCode                          submitPrimaryCommandList(ID3D12GraphicsCommandList* pCommandList);                

    // Binds the pipeline state. Must be called after a Root signature call!!
    void                                bindPipeline(ID3D12GraphicsCommandList* list, ContextState& state);

    ResultCode                          createCommandList(D3D12PrimaryCommandList** ppList, GraphicsQueueTypeFlags flags);
    ResultCode                          destroyCommandList(D3D12PrimaryCommandList* pList);
    ShaderVisibleDescriptorTable        uploadToShaderVisible(CpuDescriptorTable table, GpuHeapType shaderVisibleType);
    void                                bindCurrentResources();
    void                                internalBindVertexBuffersAndIndexBuffer(ID3D12GraphicsCommandList* list, ContextState& state);

    // Bind the root signature, must be called before bindPipeline!
    void                                bindRootSignature(ID3D12GraphicsCommandList* pList, ContextState& state);

    D3D12Device*                        m_pDevice;
    std::vector<ContextFrame>           m_contextFrames;
    U32                                 m_currentBufferIndex;
    U32                                 m_bufferCount;
    ContextDirtyFlags                   m_dirtyFlags;    
    D3D12PrimaryCommandList*            m_pPrimaryCommandList;
    std::vector<ContextState>           m_contextStates;
    std::vector<D3D12_RESOURCE_BARRIER> m_barrierTransitions;
    D3D12Queue*                         m_queue;
};

class D3D12Device : public GraphicsDevice 
{
public:
    D3D12Device()
        : m_device(nullptr)
        , m_pAdapter(nullptr) { }

    ResultCode                          initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info);
    void                                destroy();

    ID3D12Device*                       get() const { return m_device; }
    D3D12Adapter*                       getAdapter() const { return m_pAdapter; }

    ResultCode                          destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    D3D12Queue                          createCommandQueue(D3D12_COMMAND_LIST_TYPE type);
    ResultCode                          destroyCommandQueue(D3D12Queue& pQueue);
    D3D12Queue*                         getQueue(D3D12_COMMAND_LIST_TYPE type) { auto& iter = m_queues.find(type); if (iter != m_queues.end()) return &iter->second; return nullptr; }

    GraphicsContext*                    createContext() override;
    ResultCode                          releaseContext(GraphicsContext* pContext) override;

    ResultCode                          reserveMemory(const MemoryReserveDescription& desc) override;
    GraphicsSwapchain*                  createSwapchain(const SwapchainCreateDescription& desciption, void* windowHandle) override;

    // Helper descriptor creators for the device.
    void                                createSampler(const D3D12_SAMPLER_DESC& desc);
    ResultCode                          createResource(GraphicsResource** ppResource, const GraphicsResourceDescription& description, ResourceState initState) override;
    ResultCode                          destroyResource(GraphicsResource* pResource) override;
    void                                copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, const CopyBufferRegion* regions, U32 numRegions) override;
    void                                copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    D3D12_FEATURE_DATA_FORMAT_SUPPORT   checkFormatSupport(ResourceFormat format);
    DescriptorHeapAllocationManager*    getDescriptorHeapManager() { return &m_descHeapManager; }

    ResultCode                          loadShaderProgram
                                            (
                                                ShaderProgramId program, 
                                                ShaderProgramPermutation permutation, 
                                                const ShaderProgramDefinition& definition
                                            ) override;

    ResultCode                          unloadShaderProgram(ShaderProgramId program) override;
    void                                unloadAllShaderPrograms() override;

    Bool                                makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) override;
    Bool                                destroyVertexLayout(VertexInputLayoutId id) override;

    D3D12ResourceAllocationManager*     resourceAllocationManager() { return &m_resourceAllocationManager; }

private:
    void                                createCommandQueues();
    void                                destroyCommandQueues();
    // Resource pools.
    D3D12ResourceAllocationManager      m_resourceAllocationManager;
    ID3D12Device*                       m_device;
    D3D12Adapter*                       m_pAdapter;

    DescriptorHeapAllocationManager     m_descHeapManager;
    std::map<D3D12_COMMAND_LIST_TYPE, D3D12Queue> m_queues;
};
} // Recluse