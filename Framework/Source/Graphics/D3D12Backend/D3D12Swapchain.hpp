//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"


namespace Recluse {

class D3D12Device;

class D3D12Swapchain : public GraphicsSwapchain {
public:
    D3D12Swapchain(const SwapchainCreateDescription& desc)
        : m_pSwapchain(nullptr)
        , GraphicsSwapchain(desc) { }

    ErrType initialize(D3D12Device* pDevice);
    void destroy();

    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;

private:
    IDXGISwapChain1* m_pSwapchain;
};
} // Recluse