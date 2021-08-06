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
    D3D12Instance* pInstance                = pDevice->getAdapter()->getInstance();
    IDXGIFactory2* pFactory                 = pInstance->get();
    ID3D12CommandQueue* pQueue              = static_cast<D3D12Queue*>(desc.pBackbufferQueue)->get();    
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

    result = pFactory->CreateSwapChainForHwnd(pQueue, windowHandle, &swapchainDesc, nullptr, nullptr, &swapchain1);

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

    m_maxFrames = desc.desiredFrames;

    return REC_RESULT_OK;
}


void D3D12Swapchain::destroy()
{
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
    HRESULT result = S_OK;
    result = m_pSwapchain->Present(0, 0);
    if (FAILED(result)) {

        R_ERR(R_CHANNEL_D3D12, "Failed to present current frame: %d");        

    }

    incrementFrameIndex();

    m_currentBackbufferIndex = m_pSwapchain->GetCurrentBackBufferIndex();

    return REC_RESULT_OK;
}
} // Recluse