//
#include "D3D12Device.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Context.hpp"
#include "D3D12Queue.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12Swapchain::initialize(D3D12Device* pDevice)
{
    R_DEBUG(R_CHANNEL_D3D12, "Createing dxgi swapchain...");

    const SwapchainCreateDescription& desc  = getDesc();
    D3D12Context* pContext                  = pDevice->getAdapter()->getContext();
    IDXGIFactory2* pFactory                 = pContext->get();
    ID3D12CommandQueue* pQueue              = static_cast<D3D12Queue*>(desc.pBackbufferQueue)->get();    
    HWND windowHandle                       = pDevice->getWindowHandle();
    HRESULT result                          = S_OK;
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc     = { };

    swapchainDesc.Width                     = desc.renderWidth;
    swapchainDesc.Height                    = desc.renderHeight;
    swapchainDesc.BufferCount               = desc.desiredFrames;
    swapchainDesc.BufferUsage               = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.Format                    = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapchainDesc.AlphaMode                 = DXGI_ALPHA_MODE_IGNORE;
    swapchainDesc.Scaling                   = DXGI_SCALING_STRETCH;
    swapchainDesc.Stereo                    = FALSE;
    swapchainDesc.SwapEffect                = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count          = 1;
    swapchainDesc.SampleDesc.Quality        = 0;

    pFactory->CreateSwapChainForHwnd(pQueue, windowHandle, &swapchainDesc, nullptr, nullptr, &m_pSwapchain);

    if (result != S_OK) {
    
        R_ERR(R_CHANNEL_D3D12, "Failed to create d3d12 swapchain!");
        
        return REC_RESULT_FAILED;
    }

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
} // Recluse