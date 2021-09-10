//
#include "D3D12Device.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Instance.hpp"
#include "D3D12Queue.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12Swapchain::initialize(D3D12Device* pDevice)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating dxgi swapchain...");
    HWND windowHandle = pDevice->getWindowHandle();

    if (!windowHandle) {

        R_ERR(R_CHANNEL_D3D12, "Can not create swapchain without a window handle!");

        return REC_RESULT_FAILED;
    }

    const SwapchainCreateDescription& desc  = getDesc();
    D3D12Queue* pQueue                      = pDevice->getBackbufferQueue();
    D3D12Instance* pInstance                = pDevice->getAdapter()->getInstance();
    IDXGIFactory2* pFactory                 = pInstance->get();
    ID3D12CommandQueue* pNativeQueue        = pQueue->get();    
    HRESULT result                          = S_OK;
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc     = { };
    IDXGISwapChain1* swapchain1             = nullptr;

    swapchainDesc.Width                     = desc.renderWidth;
    swapchainDesc.Height                    = desc.renderHeight;
    swapchainDesc.BufferCount               = desc.desiredFrames;
    swapchainDesc.BufferUsage               = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.AlphaMode                 = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Scaling                   = DXGI_SCALING_STRETCH;
    swapchainDesc.Stereo                    = FALSE;
    swapchainDesc.SwapEffect                = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count          = 1;
    swapchainDesc.SampleDesc.Quality        = 0;

    result = pFactory->CreateSwapChainForHwnd(pNativeQueue, windowHandle, &swapchainDesc, 
        nullptr, nullptr, &swapchain1);

    if (result != S_OK) {
    
        R_ERR(R_CHANNEL_D3D12, "Failed to create d3d12 swapchain!");
        
        return REC_RESULT_FAILED;
    }

    result = swapchain1->QueryInterface<IDXGISwapChain3>(&m_pSwapchain);
    swapchain1->Release();

    if (FAILED(result)) {
        
        R_ERR(R_CHANNEL_D3D12, "Could not query for swapchain3 interface!");

        destroy();

        return REC_RESULT_FAILED;
    }

    m_maxFrames         = desc.desiredFrames;
    m_pDevice           = pDevice;
    m_pBackbufferQueue  = pQueue;

    // Initialize our frame resources, make sure to assign device and other values before calling this.
    initializeFrameResources();

    return REC_RESULT_OK;
}


void D3D12Swapchain::destroy()
{
    destroyFrameResources();

    if (m_pSwapchain) {
    
        R_DEBUG(R_CHANNEL_D3D12, "Destroying swapchain...");

        m_pSwapchain->Release();
        m_pSwapchain = nullptr;

    }
}


GraphicsResource* D3D12Swapchain::getFrame(U32 idx)
{
    return nullptr;
}


GraphicsResourceView* D3D12Swapchain::getFrameView(U32 idx)
{
    return nullptr;
}


ErrType D3D12Swapchain::present()
{
    ID3D12CommandQueue* pQueue  = m_pBackbufferQueue->get();
    ID3D12Fence* pFence         = nullptr;
    HRESULT result              = S_OK;
    U64 currentFenceValue       = 0;
    HANDLE eventHandle          = NULL;

    result = m_pSwapchain->Present(0, 0);
    
    if (FAILED(result)) {

        R_ERR(R_CHANNEL_D3D12, "Failed to present current frame: %d");        

    }

    eventHandle         = m_frameResources[m_currentFrameIndex].pEvent;
    pFence              = m_frameResources[m_currentFrameIndex].pFence;
    currentFenceValue   = m_frameResources[m_currentFrameIndex].fenceValue;        


    // Set the value for the fence on GPU side, letting us know when our commands have finished.
    pQueue->Signal(pFence, currentFenceValue);

    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();

    if (pFence->GetCompletedValue() < currentFenceValue) {
    
        pFence->SetEventOnCompletion(currentFenceValue, eventHandle);
        WaitForSingleObjectEx(eventHandle, INFINITE, false);
    
    }

    m_frameResources[m_currentFrameIndex].fenceValue = currentFenceValue + 1;

    m_pDevice->incrementBufferIndex();
    m_pDevice->resetCurrentResources();

    return REC_RESULT_OK;
}


ErrType D3D12Swapchain::initializeFrameResources()
{
    HRESULT result          = S_OK;
    ID3D12Device* pDevice   = m_pDevice->get();

    m_frameResources.resize(m_maxFrames);
    
    for (U32 i = 0; i < m_frameResources.size(); ++i) {
    
        FrameResource& frameRes = m_frameResources[i];

        frameRes.fenceValue = 0;
        frameRes.pEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        result = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&frameRes.pFence);

        if (FAILED(result)) {
            R_ERR(R_CHANNEL_D3D12, "Failed to initialize a fence for frame resource: %d", i);
        }

    }
    
    return REC_RESULT_OK;
}


ErrType D3D12Swapchain::destroyFrameResources()
{
    if (!m_frameResources.empty()) {
    
        for (U32 i = 0; i < m_frameResources.size(); ++i) {

            CloseHandle(m_frameResources[i].pEvent);
            m_frameResources[i].pEvent = nullptr;

            m_frameResources[i].pFence->Release();
            m_frameResources[i].pFence = nullptr;
            
        }
    
        m_frameResources.clear();
    }

    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse