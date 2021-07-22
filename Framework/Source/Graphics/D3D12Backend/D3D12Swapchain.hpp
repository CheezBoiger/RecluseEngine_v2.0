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
        , m_maxFrames(0)
        , GraphicsSwapchain(desc)
        , m_currentBackbufferIndex(0) { }

    ErrType initialize(D3D12Device* pDevice);
    void destroy();

    ErrType present() override;

    GraphicsResource* getFrame(U32 idx) override;
    GraphicsResourceView* getFrameView(U32 idx) override;

private:

    void incrementFrameIndex() 
        { m_currentBackbufferIndex = (m_currentFrameIndex + 1) % m_maxFrames; }

    IDXGISwapChain3* m_pSwapchain;
    U32 m_maxFrames;
    U32 m_currentBackbufferIndex;   // Current backbuffer index notifies the current swapchain image rendering.
    U32 m_currentFrameIndex;        // Current frame index notifies current frame resource to run.
};
} // Recluse