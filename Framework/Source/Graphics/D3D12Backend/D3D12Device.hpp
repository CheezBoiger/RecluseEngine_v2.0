//
#pragma once

#include "Win32/Win32Common.hpp"
#include "D3D12Commons.hpp"
#include "D3D12DescriptorTableManager.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "D3D12PipelineState.hpp"

namespace Recluse {

class D3D12Adapter;
class D3D12ResourcePagedAllocator;
class D3D12Swapchain;
class D3D12Queue;
class D3D12PrimaryCommandList;
class D3D12Resource;
class D3D12Device;

struct BufferResources 
{
    ID3D12CommandAllocator* pAllocator;
    ID3D12Fence*            pFence;
    HANDLE                  pEvent;
    U64                     fenceValue;
};


class D3D12Context : public GraphicsContext
{
public:
    D3D12Context(D3D12Device* pDevice, U32 bufferCount)
        : m_pDevice(pDevice)
        , m_currentBufferIndex(0)
        , m_pPrimaryCommandList(nullptr)
        , m_bufferCount(bufferCount)
    {

    }

    void initialize();
    void release();

    virtual void begin() override;
    virtual void end() override;

    inline void incrementBufferIndex() 
    { 
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount; 
    }

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    void copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    void                copyBufferRegions
                            (
                                GraphicsResource* dst, 
                                GraphicsResource* src, 
                                CopyBufferRegion* pRegions, 
                                U32 numRegions
                            ) override;

    BufferResources*    getCurrentBufferResource() { return &m_bufferResources[m_currentBufferIndex]; }
    U32                 getCurrentBufferIndex() const { return m_currentBufferIndex; }

    const std::vector<BufferResources>& getBufferResources() const { return m_bufferResources; }
    void        resetCurrentResources();

private:

    void        initializeBufferResources(U32 buffering);
    void        destroyBufferResources();

    ErrType     createCommandList(D3D12PrimaryCommandList** ppList, GraphicsQueueTypeFlags flags);
    ErrType     destroyCommandList(D3D12PrimaryCommandList* pList);

    D3D12Device*                        m_pDevice;
    std::vector<BufferResources>        m_bufferResources;
    U32                                 m_currentBufferIndex;
    U32                                 m_bufferCount;
    D3D12PrimaryCommandList*            m_pPrimaryCommandList;
    Pipelines::PipelineStateObject      m_pipelineStateObject;
};

class D3D12Device : public GraphicsDevice 
{
public:
    D3D12Device()
        : m_windowHandle(nullptr)
        , m_device(nullptr)
        , m_pAdapter(nullptr)
        , m_graphicsQueue(nullptr)
        , m_swapchain(nullptr) { }

    ErrType initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info);
    void destroy();

    ID3D12Device* get() const { return m_device; }
    
    D3D12Adapter* getAdapter() const { return m_pAdapter; }

    ErrType createSwapchain(D3D12Swapchain** ppSwapchain, const SwapchainCreateDescription& desc);
    ErrType destroySwapchain(D3D12Swapchain* pSwapchain);

    ErrType createCommandQueue(D3D12Queue** ppQueue, GraphicsQueueTypeFlags type);
    ErrType destroyCommandQueue(D3D12Queue* pQueue);

    GraphicsContext* getContext() override { return m_context; }
    HWND getWindowHandle() const { return m_windowHandle; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    D3D12Queue* getBackbufferQueue() const { return m_graphicsQueue; }
    GraphicsSwapchain* getSwapchain() override;

    // Helper descriptor creators for the device.
    void createSampler(const D3D12_SAMPLER_DESC& desc);
    void createRenderTargetView(D3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptorHandle);
    void createDepthStencilView(D3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptorHandle);
    void createShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
    void createUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

    D3D12_FEATURE_DATA_FORMAT_SUPPORT checkFormatSupport(ResourceFormat format);

    DescriptorHeapAllocationManager* getDescriptorHeapManager() { return &m_descHeapManager; }

    ErrType loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition) override;
    ErrType unloadShaderProgram(ShaderProgramId program) override;
    void unloadAllShaderPrograms() override;

    Bool makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) override;
    Bool destroyVertexLayout(VertexInputLayoutId id) override;

    D3D12ResourcePagedAllocator* getBufferAllocator(ResourceMemoryUsage usage) const { return m_bufferPool[usage]; }
    D3D12ResourcePagedAllocator* getTextureAllocator() const { return m_texturePool; }

private:

    void initializeAllocators();
    void destroyAllocators();
    void allocateMemoryPool(D3D12MemoryPool* pPool, ResourceMemoryUsage memUsage);

    // Resource pools.
    D3D12MemoryPool                 m_bufferMemPools[ResourceMemoryUsage_Count];
    D3D12ResourcePagedAllocator*         m_bufferPool[ResourceMemoryUsage_Count];
    D3D12ResourcePagedAllocator*         m_texturePool;
    D3D12MemoryPool                 m_textureMemPool;

    ID3D12Device*                   m_device;
    D3D12Adapter*                   m_pAdapter;
    D3D12Queue*                     m_graphicsQueue;
    D3D12Swapchain*                 m_swapchain;

    DescriptorHeapAllocationManager m_descHeapManager;
    D3D12Context*                   m_context;

    HWND m_windowHandle;
};
} // Recluse