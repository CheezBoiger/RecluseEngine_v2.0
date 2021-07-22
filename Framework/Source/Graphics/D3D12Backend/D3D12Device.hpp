//
#pragma once

#include "Win32/Win32Common.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class D3D12Adapter;
class D3D12Swapchain;

class D3D12Device : public GraphicsDevice {
public:
    D3D12Device()
        : m_windowHandle(nullptr)
        , m_device(nullptr)
        , m_pAdapter(nullptr) { }

    ErrType initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info);
    void destroy();

    ID3D12Device* get() const { return m_device; }
    
    D3D12Adapter* getAdapter() const { return m_pAdapter; }

    ErrType createSwapchain(GraphicsSwapchain** ppSwapchain, const SwapchainCreateDescription& desc) override;
    ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) override;

    ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) override;
    ErrType destroyCommandQueue(GraphicsQueue* pQueue) override;

    HWND getWindowHandle() const { return m_windowHandle; }

private:
    ID3D12Device* m_device;
    D3D12Adapter* m_pAdapter;
    HWND m_windowHandle;
};
} // Recluse