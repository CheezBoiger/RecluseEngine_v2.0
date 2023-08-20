//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"

namespace Recluse {

class D3D12Device;
class D3D12Queue;

struct FrameResource {
    D3D12Resource*              frameResource;
    BufferResources             bufferResources;
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
        , m_pBackbufferQueue(pBackbufferQueue) { }

    ResultCode initialize(D3D12Device* pDevice);
    void destroy();

    // Present to our window display.
    ResultCode              present(PresentConfig config = PresentConfig_Present) override;
    ResultCode              onRebuild() override;

    GraphicsResource*       getFrame(U32 idx) override;

    ResultCode              submitPrimaryCommandList(ID3D12GraphicsCommandList* pCommandList);
    ResultCode              prepareNextFrame();

    U64                     getCurrentFenceValue() { return m_frameResources[m_currentFrameIndex].bufferResources.fenceValue; }
    U32                     getCurrentFrameIndex() override { return m_currentFrameIndex; }

private:

    ResultCode              initializeFrameResources();
    ResultCode              destroyFrameResources();
    
    IDXGISwapChain3*            m_pSwapchain;
    D3D12Device*                m_pDevice;
    U32                         m_maxFrames;
    U32                         m_currentFrameIndex;        // Current frame index notifies current frame resource to run.
    std::vector<FrameResource>  m_frameResources;
    D3D12Queue*                 m_pBackbufferQueue;
};
} // Recluse