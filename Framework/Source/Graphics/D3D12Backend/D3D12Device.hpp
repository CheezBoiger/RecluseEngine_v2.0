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

struct BufferResources {
    ID3D12CommandAllocator* pAllocator;
};

class D3D12Device : public GraphicsDevice {
public:
    D3D12Device()
        : m_windowHandle(nullptr)
        , m_device(nullptr)
        , m_pAdapter(nullptr)
        , m_currentBufferIndex(0)
        , m_bufferCount(0) { }

    ErrType initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info);
    void destroy();

    ID3D12Device* get() const { return m_device; }
    
    D3D12Adapter* getAdapter() const { return m_pAdapter; }

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, const SwapchainCreateDescription& desc) override;
    ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) override;
    ErrType destroyCommandQueue(GraphicsQueue* pQueue) override;

    HWND getWindowHandle() const { return m_windowHandle; }

    ErrType reserveMemory(const MemoryReserveDesc& desc) override;

    U32 getCurrentBufferIndex() const { return m_currentBufferIndex; }

    const std::vector<BufferResources>& getBufferResources() const { return m_bufferResources; }

    void resetCurrentResources();
    
    inline void incrementBufferIndex() 
        { m_currentBufferIndex = (m_currentBufferIndex + 1) % m_bufferCount; }

private:

    void initializeBufferResources(U32 buffering);
    void destroyBufferResources();

    // Resource pools.
    D3D12Allocator* m_bufferPool[RESOURCE_MEMORY_USAGE_COUNT];
    D3D12Allocator* m_texturePool;

    ID3D12Device* m_device;
    D3D12Adapter* m_pAdapter;

    std::vector<BufferResources>    m_bufferResources;
    U32                             m_currentBufferIndex;
    U32                             m_bufferCount;

    HWND m_windowHandle;
};
} // Recluse