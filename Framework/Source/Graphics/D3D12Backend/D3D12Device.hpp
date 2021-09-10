//
#pragma once

#include "Win32/Win32Common.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class D3D12Adapter;
class D3D12Allocator;
class D3D12Swapchain;
class D3D12Queue;
class D3D12CommandList;

struct BufferResources {
    ID3D12CommandAllocator* pAllocator;
    ID3D12Fence*            pFence;
    HANDLE                  pEvent;
    U64                     fenceValue;
};

class D3D12Device : public GraphicsDevice {
public:
    D3D12Device()
        : m_windowHandle(nullptr)
        , m_device(nullptr)
        , m_pAdapter(nullptr)
        , m_currentBufferIndex(0)
        , m_bufferCount(0)
        , m_graphicsQueue(nullptr)
        , m_pPrimaryCommandList(nullptr)
        , m_swapchain(nullptr) { }

    ErrType initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info);
    void destroy();

    ID3D12Device* get() const { return m_device; }
    
    D3D12Adapter* getAdapter() const { return m_pAdapter; }

    ErrType createSwapchain(D3D12Swapchain** ppSwapchain, const SwapchainCreateDescription& desc);
    ErrType destroySwapchain(D3D12Swapchain* pSwapchain);

    ErrType createCommandQueue(D3D12Queue** ppQueue, GraphicsQueueTypeFlags type);
    ErrType destroyCommandQueue(D3D12Queue* pQueue);

    ErrType createCommandList(D3D12CommandList** ppList, GraphicsQueueTypeFlags flags);
    ErrType destroyCommandList(D3D12CommandList* pList);

    HWND getWindowHandle() const { return m_windowHandle; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    U32 getCurrentBufferIndex() const { return m_currentBufferIndex; }

    const std::vector<BufferResources>& getBufferResources() const { return m_bufferResources; }

    void resetCurrentResources();
    
    inline void incrementBufferIndex() 
        { m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount; }

    D3D12Queue* getBackbufferQueue() const { return m_graphicsQueue; }

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) override;

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    ErrType copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
        CopyBufferRegion* pRegions, U32 numRegions) override;

    GraphicsCommandList* getCommandList() override;
    GraphicsSwapchain* getSwapchain() override;

    BufferResources* getCurrentBufferResource() { return &m_bufferResources[m_currentBufferIndex]; }

private:

    void initializeBufferResources(U32 buffering);
    void destroyBufferResources();

    // Resource pools.
    D3D12Allocator*                 m_bufferPool[RESOURCE_MEMORY_USAGE_COUNT];
    D3D12Allocator*                 m_texturePool;

    ID3D12Device*                   m_device;
    D3D12Adapter*                   m_pAdapter;
    D3D12Queue*                     m_graphicsQueue;
    D3D12CommandList*               m_pPrimaryCommandList;
    D3D12Swapchain*                 m_swapchain;

    std::vector<BufferResources>    m_bufferResources;
    U32                             m_currentBufferIndex;
    U32                             m_bufferCount;

    HWND m_windowHandle;
};
} // Recluse