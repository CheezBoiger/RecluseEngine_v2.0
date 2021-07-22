//
#include "D3D12Device.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Swapchain.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12Device::initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating D3D12 device...");

    IDXGIAdapter* pAdapter  = adapter->get();
    HRESULT result          = S_OK;

    result = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&m_device);


    if (result != S_OK) {
    
        return REC_RESULT_FAILED;
    
    }

    m_pAdapter = adapter;

    if (info.winHandle) {
        m_windowHandle = (HWND)info.winHandle;
    }

    R_DEBUG(R_CHANNEL_D3D12, "Successfully created D3D12 device!");
    
    return REC_RESULT_OK;
}


void D3D12Device::destroy()
{
    if (m_device) {
    
        R_DEBUG(R_CHANNEL_D3D12, "Destroying D3D12 device...");

        m_device->Release();
    
    }
}


ErrType D3D12Device::createSwapchain(GraphicsSwapchain** ppSwapchain, const SwapchainCreateDescription& desc)
{
    ErrType result = REC_RESULT_OK;
    D3D12Swapchain* pSwapchain = new D3D12Swapchain(desc);

    result = pSwapchain->initialize(this);

    if (result != REC_RESULT_OK) {

        R_ERR(R_CHANNEL_D3D12, "Failed to create d3d swapchain!");

        pSwapchain->destroy();
        delete pSwapchain;
        return result;
    }

    *ppSwapchain = pSwapchain;

    return result;
}


ErrType D3D12Device::destroySwapchain(GraphicsSwapchain* pSwapchain)
{
    ErrType result = REC_RESULT_OK;
    D3D12Swapchain* pD3D12Swapchain = static_cast<D3D12Swapchain*>(pSwapchain);

    pD3D12Swapchain->destroy();
    delete pD3D12Swapchain;
   
    return result;
}


ErrType D3D12Device::createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type)
{
    D3D12Queue* pD3D12Queue = new D3D12Queue(type);
    ErrType result          = REC_RESULT_OK;

    result = pD3D12Queue->initialize(this);

    if (result != REC_RESULT_OK) {
    
        R_ERR(R_CHANNEL_D3D12, "Failed to create d3d12 queue!");

        pD3D12Queue->destroy();
        delete pD3D12Queue;

        return result;    

    }

    *ppQueue = pD3D12Queue;

    return result;
}


ErrType D3D12Device::destroyCommandQueue(GraphicsQueue* pQueue)
{
    if (!pQueue) {

        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    D3D12Queue* pD3D12Queue = static_cast<D3D12Queue*>(pQueue);

    pD3D12Queue->destroy();

    delete pD3D12Queue;

    return REC_RESULT_OK;
}
} // Recluse