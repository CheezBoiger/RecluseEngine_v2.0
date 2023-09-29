//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"

namespace Recluse {
namespace D3D12 {
class D3D12Device;
class D3D12Queue;
class D3D12Context;

struct FrameResource {
    D3D12Resource*              frameResource;
};

class D3D12Swapchain : public GraphicsSwapchain 
{
public:
    D3D12Swapchain(const SwapchainCreateDescription& desc, D3D12Queue* pBackbufferQueue)
        : m_pSwapchain(nullptr)
        , m_maxFrames(0)
        , GraphicsSwapchain(desc)
        , m_pDevice(nullptr)
        , m_currentFrameIndex(0)
        , m_hwnd(NULL)
        , m_flags(0)
        , m_pBackbufferQueue(pBackbufferQueue) { }

    ResultCode initialize(D3D12Device* pDevice, HWND windowHandle);
    void destroy();

    // Present to our window display.
    ResultCode              prepare(GraphicsContext* context) override;
    ResultCode              present(GraphicsContext* context) override;
    ResultCode              onRebuild() override;

    GraphicsResource*       getFrame(U32 idx) override;

    // Run the basic frame sync.
    //ResultCode              prepareNextFrame();
    // Override the frame fence sync with your own buffer value sync. This could help against buffer resources 
    // needing to be lower than the frame sync.
    ResultCode              prepareNextFrameOverride(U64 currentBufferFenceValue, U64& nextBufferFenceValue);

    U64                     getCurrentCompletedValue();
    U32                     getCurrentFrameIndex() override { return m_currentFrameIndex; }

private:
    ResultCode                  initializeFrameResources();
    ResultCode                  destroyFrameResources();
    
    IDXGISwapChain3*            m_pSwapchain;
    D3D12Device*                m_pDevice;
    U32                         m_maxFrames;
    U32                         m_currentFrameIndex;        // Current frame index notifies current frame resource to run.
    std::vector<FrameResource>  m_frameResources;
    D3D12Queue*                 m_pBackbufferQueue;
    UINT                        m_flags;
    HWND                        m_hwnd;
};
} // D3D12
} // Recluse