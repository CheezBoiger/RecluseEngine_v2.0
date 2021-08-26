//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"


namespace Recluse {

class D3D12Device;
class D3D12Queue;

struct FrameResource {
    ID3D12Fence*    pFence;
    HANDLE          pEvent;
    U64             fenceValue;
};

class D3D12Swapchain : public GraphicsSwapchain {
public:
    D3D12Swapchain(const SwapchainCreateDescription& desc)
        : m_pSwapchain(nullptr)
        , m_maxFrames(0)
        , GraphicsSwapchain(desc)
        , m_pDevice(nullptr)
        , m_pBackbufferQueue(nullptr) { }

    ErrType initialize(D3D12Device* pDevice);
    void destroy();

    // Present to our window display.
    ErrType present() override;

    GraphicsResource*       getFrame(U32 idx) override;
    GraphicsResourceView*   getFrameView(U32 idx) override;

private:

    ErrType initializeFrameResources();
    ErrType destroyFrameResources();
    
    IDXGISwapChain3*            m_pSwapchain;
    D3D12Device*                m_pDevice;
    U32                         m_maxFrames;
    U32                         m_currentFrameIndex;        // Current frame index notifies current frame resource to run.
    std::vector<FrameResource>  m_frameResources;
    D3D12Queue*                 m_pBackbufferQueue;
};
} // Recluse