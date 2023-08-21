//
#include "D3D12Device.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Instance.hpp"
#include "D3D12Queue.hpp"
#include "D3D12CommandList.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ResultCode D3D12Swapchain::initialize(D3D12Device* pDevice)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating dxgi swapchain...");
    HWND windowHandle = pDevice->getWindowHandle();

    if (!windowHandle) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Can not create swapchain without a window handle!");

        return RecluseResult_Failed;
    }

    const SwapchainCreateDescription& desc  = getDesc();
    D3D12Queue* pQueue                      = pDevice->getBackbufferQueue();
    D3D12Instance* pInstance                = pDevice->getAdapter()->getInstance();
    IDXGIFactory2* pFactory                 = pInstance->get();
    ID3D12CommandQueue* pNativeQueue        = pQueue->get();    
    HRESULT result                          = S_OK;
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc     = { };
    IDXGISwapChain1* swapchain1             = nullptr;
    DXGI_FORMAT format                      = Dxgi::getNativeFormat(desc.format);

    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = pDevice->checkFormatSupport(desc.format);
        if (!(formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DISPLAY)) 
        {
            R_WARN(R_CHANNEL_D3D12, "Swapchain format not supported for display, trying default...");
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
    }

    swapchainDesc.Width                     = desc.renderWidth;
    swapchainDesc.Height                    = desc.renderHeight;
    swapchainDesc.BufferCount               = desc.desiredFrames;
    swapchainDesc.BufferUsage               = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.Format                    = format;
    swapchainDesc.AlphaMode                 = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Scaling                   = DXGI_SCALING_STRETCH;
    swapchainDesc.Stereo                    = FALSE;
    swapchainDesc.SwapEffect                = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count          = 1;
    swapchainDesc.SampleDesc.Quality        = 0;

    result = pFactory->CreateSwapChainForHwnd
                            (
                                pNativeQueue, 
                                windowHandle, 
                                &swapchainDesc, 
                                nullptr, 
                                nullptr, 
                                &swapchain1
                            );

    if (result != S_OK) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create d3d12 swapchain!");
        
        return RecluseResult_Failed;
    }

    result = swapchain1->QueryInterface<IDXGISwapChain3>(&m_pSwapchain);
    swapchain1->Release();

    if (FAILED(result)) 
    {    
        R_ERROR(R_CHANNEL_D3D12, "Could not query for swapchain3 interface!");

        destroy();

        return RecluseResult_Failed;
    }

    m_maxFrames         = desc.desiredFrames;
    m_pDevice           = pDevice;
    m_pBackbufferQueue  = pQueue;

    // Initialize our frame resources, make sure to assign device and other values before calling this.
    initializeFrameResources();

    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();

    m_frameResources[m_currentFrameIndex].fenceValue = m_pBackbufferQueue->waitForGpu(m_frameResources[m_currentFrameIndex].fenceValue);
    
    return RecluseResult_Ok;
}


void D3D12Swapchain::destroy()
{
    destroyFrameResources();

    if (m_pSwapchain) 
    { 
        R_DEBUG(R_CHANNEL_D3D12, "Destroying swapchain...");

        m_pSwapchain->Release();
        m_pSwapchain = nullptr;
    }


}


ResultCode D3D12Swapchain::onRebuild()
{
    // Destroy the current swapchain and recreate it with the new descriptions.
    m_currentFrameIndex = 0;
    destroy();
    return initialize(m_pDevice);
}


GraphicsResource* D3D12Swapchain::getFrame(U32 idx)
{
    R_ASSERT(idx < m_frameResources.size());
    return m_frameResources[idx].frameResource;
}


ResultCode D3D12Swapchain::present(PresentConfig config)
{
    const SwapchainCreateDescription& desc = getDesc();
    UINT syncInterval           = 0;
    HRESULT result              = S_OK;

    if (desc.buffering == FrameBuffering_Double)
        syncInterval = 1;

    result = m_pSwapchain->Present(syncInterval, 0);
    
    if (FAILED(result)) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to present current frame: %d", getCurrentFrameIndex());        
    }

    return RecluseResult_Ok;
}


ResultCode D3D12Swapchain::initializeFrameResources()
{
    HRESULT result          = S_OK;
    ID3D12Device* pDevice   = m_pDevice->get();
    const SwapchainCreateDescription& swapchainDesc = getDesc();

    m_frameResources.resize(m_maxFrames);
    
    for (U32 i = 0; i < m_frameResources.size(); ++i) 
    {
        FrameResource& frameRes = m_frameResources[i];
        // We don't need to initialize frame resources, they are already 
        // handled by the swapchain. We just wrap them!
        ID3D12Resource* pFrameResource      = nullptr;
        GraphicsResourceDescription desc    = { };
        desc.dimension                      = ResourceDimension_2d;
        desc.depthOrArraySize               = 1;
        desc.height                         = swapchainDesc.renderHeight;
        desc.width                          = swapchainDesc.renderWidth;
        desc.mipLevels                      = 1;
        desc.name                           = "Swapchain Frame Resource";
        desc.samples                        = 1;
        desc.format                         = swapchainDesc.format;
        Bool result = SUCCEEDED(m_pSwapchain->GetBuffer(i, __uuidof(ID3D12Resource*), (void**)&pFrameResource));
        R_ASSERT(result);
        m_frameResources[i].frameResource = new D3D12Resource(m_pDevice, pFrameResource, ResourceState_Present);
        m_frameResources[i].frameResource->generateId();
        m_frameResources[i].fenceValue = 0;
    }

    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
    
    return RecluseResult_Ok;
}


ResultCode D3D12Swapchain::destroyFrameResources()
{
    if (!m_frameResources.empty()) 
    {
        for (U32 i = 0; i < m_frameResources.size(); ++i) 
        {
            // We just delete the resource, it just wraps the swapchain frame.
            // We call the native destroy() function, as the swapchain owns it.
            m_frameResources[i].frameResource->get()->Release();
            delete m_frameResources[i].frameResource;
        }
    
        m_frameResources.clear();
    }

    return RecluseResult_Ok;
}


ResultCode D3D12Swapchain::submitPrimaryCommandList(ID3D12GraphicsCommandList* pCommandList)
{
    ID3D12CommandQueue* pPresentationQueue = m_pBackbufferQueue->get();
    ID3D12CommandList* pLists[] = { pCommandList };

    R_ASSERT(pPresentationQueue != NULL);
    R_ASSERT(pCommandList != NULL);

    pPresentationQueue->ExecuteCommandLists(1, pLists);
    return RecluseResult_Ok;
}


ResultCode D3D12Swapchain::prepareNextFrame()
{
    ID3D12Fence* fence = m_pBackbufferQueue->getFence();
    HANDLE e = m_pBackbufferQueue->getEvent();
    // Set the value for the fence on GPU side, letting us know when our commands have finished.
    const U64 currentFenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
    m_pBackbufferQueue->get()->Signal(fence, currentFenceValue);

    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
 
    U64& newFrameFenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
    const U64 completedValue = fence->GetCompletedValue();
    if (completedValue < newFrameFenceValue)
    {
        fence->SetEventOnCompletion(newFrameFenceValue, e);
        WaitForSingleObject(e, INFINITE);
    }
    newFrameFenceValue = currentFenceValue + 1;
    return RecluseResult_Ok;
}


ResultCode D3D12Swapchain::prepareNextFrameOverride(U64 currentBufferFenceValue, U64& nextBufferFenceValue)
{
    ID3D12Fence* fence = m_pBackbufferQueue->getFence();
    HANDLE e = m_pBackbufferQueue->getEvent();
    const U64 currentFenceValue = currentBufferFenceValue;
    m_pBackbufferQueue->get()->Signal(fence, currentFenceValue);

    // We still need to query for our next frame, as it is essential, but overall we wait by buffer instead of frame index.
    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
 
    U64& newFrameFenceValue = nextBufferFenceValue;
    const U64 completedValue = fence->GetCompletedValue();
    if (completedValue < newFrameFenceValue)
    {
        fence->SetEventOnCompletion(newFrameFenceValue, e);
        WaitForSingleObject(e, INFINITE);
    }
    newFrameFenceValue = currentFenceValue + 1;
    return RecluseResult_Ok;
}


U64 D3D12Swapchain::getCurrentCompletedValue()
{
    return m_pBackbufferQueue->getFence()->GetCompletedValue(); 
}
} // Recluse