//
#include "D3D12Device.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Instance.hpp"
#include "D3D12Queue.hpp"
#include "D3D12CommandList.hpp"
#include "Recluse/Messaging.hpp"

#include "../../Win32/IO/Win32Window.hpp"

namespace Recluse {
namespace D3D12 {


Bool shouldFullscreen(HWND handle)
{
    Window* pWindow = getWindowAssociatedWithHwnd(handle);
    if (pWindow)
    {
        if (pWindow->isFullscreen() && !pWindow->isBorderless())
        {
            // Disable allow tearing, as it only works in windowed mode.
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}


UINT setFullscreenState(IDXGISwapChain* swapchain, Bool fullscreen, UINT currentFlags)
{
    UINT flags = currentFlags;
    BOOL isAlreadyInFullscreenState = false;
    swapchain->GetFullscreenState(&isAlreadyInFullscreenState, nullptr);
    if (fullscreen)
    {
        // Disable allow tearing, as it only works in windowed mode.
        if (!isAlreadyInFullscreenState)
        {
            swapchain->SetFullscreenState(true, nullptr);
        }
        flags &= ~(DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
    }
    else
    {
        if (isAlreadyInFullscreenState)
        {
            swapchain->SetFullscreenState(false, nullptr);
        }
    }
    return flags;
}


ResultCode D3D12Swapchain::initialize(D3D12Device* pDevice, HWND windowHandle)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating dxgi swapchain...");

    if (!windowHandle) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Can not create swapchain without a window handle!");

        return RecluseResult_Failed;
    }

    const SwapchainCreateDescription& desc  = getDesc();
    D3D12Queue* pQueue                      = pDevice->getQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
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

    // Per D3D12 instruction, we are not allowed to use our backbuffers for Unordered Access.
    // This is an intentional design rework for D3D12. D3D11 will still allow it, but it is safe practice to 
    // not use our backbuffers for unordered access views.
    // Link to discussion:
    // https://gamedev.net/forums/topic/673687-d3d12-use-compute-shader-to-write-to-swap-chain-backbuffer/
    swapchainDesc.Width                     = desc.renderWidth;
    swapchainDesc.Height                    = desc.renderHeight;
    swapchainDesc.BufferCount               = desc.desiredFrames;
    swapchainDesc.BufferUsage               = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    swapchainDesc.Format                    = format;
    swapchainDesc.AlphaMode                 = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Scaling                   = DXGI_SCALING_STRETCH;
    swapchainDesc.Stereo                    = FALSE;
    swapchainDesc.SwapEffect                = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count          = 1;
    swapchainDesc.SampleDesc.Quality        = 0;
    swapchainDesc.Flags                     = 0;

    if (pInstance->hasTearingSupport())
    {
        swapchainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }
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
    m_flags             = swapchainDesc.Flags;
    m_hwnd              = windowHandle;

    if (shouldFullscreen(windowHandle))
    {
        m_flags = setFullscreenState(m_pSwapchain, true, m_flags);
        m_pSwapchain->ResizeBuffers(swapchainDesc.BufferCount, swapchainDesc.Width, swapchainDesc.Height, swapchainDesc.Format, swapchainDesc.Flags);
    }
    // Initialize our frame resources, make sure to assign device and other values before calling this.
    initializeFrameResources();

    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
    return RecluseResult_Ok;
}


void D3D12Swapchain::destroy()
{
    destroyFrameResources();

    if (m_pSwapchain) 
    { 
        R_DEBUG(R_CHANNEL_D3D12, "Destroying swapchain...");
        BOOL isFullscreen = false;
        m_pSwapchain->GetFullscreenState(&isFullscreen, nullptr);
        if (isFullscreen)
        {
            m_pSwapchain->SetFullscreenState(false, nullptr);
        }
        m_pSwapchain->Release();
        m_pSwapchain = nullptr;
    }


}


ResultCode D3D12Swapchain::onRebuild()
{
    R_DEBUG(R_CHANNEL_D3D12, "Rebuilding swapchain buffer.");
    // Destroy the current swapchain and recreate it with the new descriptions.
    m_currentFrameIndex                             = 0;
    const SwapchainCreateDescription& description   = getDesc();
    UINT swapchainFlags                             = m_flags;

    // Destroy the current d3d12 swapchain resources.
    destroyFrameResources();

    if (m_pDevice->getAdapter()->getInstance()->hasTearingSupport())
    {
        swapchainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }

    // Set the fullscreen state if we need to.
    m_flags = swapchainFlags;
    m_flags = setFullscreenState(m_pSwapchain, shouldFullscreen(m_hwnd), m_flags);

    HRESULT result = m_pSwapchain->ResizeBuffers(description.desiredFrames, description.renderWidth, description.renderHeight, Dxgi::getNativeFormat(description.format), swapchainFlags);

    if (FAILED(result))
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to rebuild the swapchain!!");
        return RecluseResult_Failed;
    }

    // Rebuild the new d3d12 swapchain resource.s
    initializeFrameResources();
    return RecluseResult_Ok;
}


GraphicsResource* D3D12Swapchain::getFrame(U32 idx)
{
    R_ASSERT(idx < m_frameResources.size());
    return m_frameResources[idx].frameResource;
}


ResultCode D3D12Swapchain::prepare(GraphicsContext* context)
{
    D3D12Context* d3dContext = context->castTo<D3D12Context>();

    d3dContext->begin();
    R_ASSERT_FORMAT(d3dContext->obtainFrameCount() <= getDesc().desiredFrames, "Context frame count is higher than the actual swapchain buffer count! This will cause a crash!");

    // We still need to query for our next frame, as it is essential, but overall we wait by buffer instead of frame index.
    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
    return RecluseResult_Ok;
}


ResultCode D3D12Swapchain::present(GraphicsContext* context)
{
    const SwapchainCreateDescription& desc = getDesc();
    UINT syncInterval           = 0;
    HRESULT result              = S_OK;
    UINT flags                  = 0;
    ResultCode err              = RecluseResult_Ok;
    if (desc.buffering == FrameBuffering_Double)
        syncInterval = 1;
    if (desc.buffering == FrameBuffering_Triple && (m_flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING))
        flags |= DXGI_PRESENT_ALLOW_TEARING;

    result = m_pSwapchain->Present(syncInterval, flags);
    
    if (FAILED(result)) 
    {
        switch (result)
        {
            case DXGI_ERROR_INVALID_CALL:
                err = RecluseResult_NeedsUpdate;
                break;
            default:
                R_ERROR(R_CHANNEL_D3D12, "Failed to present current frame: %d", getCurrentFrameIndex());
                err = RecluseResult_Failed;
                break;
        }
    }

    return err;
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


//ResultCode D3D12Swapchain::prepareNextFrame()
//{
//    ID3D12Fence* fence = m_pBackbufferQueue->getFence();
//    HANDLE e = m_pBackbufferQueue->getEvent();
//    // Set the value for the fence on GPU side, letting us know when our commands have finished.
//    const U64 currentFenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
//    m_pBackbufferQueue->get()->Signal(fence, currentFenceValue);
//
//    m_currentFrameIndex = m_pSwapchain->GetCurrentBackBufferIndex();
// 
//    U64& newFrameFenceValue = m_frameResources[m_currentFrameIndex].fenceValue;
//    const U64 completedValue = fence->GetCompletedValue();
//    if (completedValue < newFrameFenceValue)
//    {
//        fence->SetEventOnCompletion(newFrameFenceValue, e);
//        WaitForSingleObject(e, INFINITE);
//    }
//    newFrameFenceValue = currentFenceValue + 1;
//    return RecluseResult_Ok;
//}


ResultCode D3D12Swapchain::prepareNextFrameOverride(U64 currentBufferFenceValue, U64& nextBufferFenceValue)
{
    ID3D12Fence* fence = m_pBackbufferQueue->getFence();
    HANDLE e = m_pBackbufferQueue->getEvent();
    const U64 currentFenceValue = currentBufferFenceValue;
    m_pBackbufferQueue->get()->Signal(fence, currentFenceValue);
 
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
} // D3D12
} // Recluse