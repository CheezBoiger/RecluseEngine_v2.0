//
#include "D3D12Device.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12CommandList.hpp"
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
        createCommandQueue(&m_graphicsQueue, QUEUE_TYPE_PRESENT);
    }

    R_DEBUG(R_CHANNEL_D3D12, "Successfully created D3D12 device!");

    initializeBufferResources(info.buffering);

    return REC_RESULT_OK;
}


void D3D12Device::destroy()
{
    if (m_graphicsQueue) {
        destroyCommandQueue(m_graphicsQueue);
        m_graphicsQueue = nullptr;
    }

    destroyBufferResources();

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


ErrType D3D12Device::createCommandQueue(D3D12Queue** ppQueue, GraphicsQueueTypeFlags type)
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


ErrType D3D12Device::destroyCommandQueue(D3D12Queue* pQueue)
{
    if (!pQueue) {

        return REC_RESULT_NULL_PTR_EXCEPTION;

    }

    D3D12Queue* pD3D12Queue = pQueue;

    pD3D12Queue->destroy();

    delete pD3D12Queue;

    return REC_RESULT_OK;
}


ErrType D3D12Device::reserveMemory(const MemoryReserveDesc& desc)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


void D3D12Device::resetCurrentResources()
{
    HRESULT result = S_OK;
    BufferResources& buffer = m_bufferResources[m_currentBufferIndex];
        
    result = buffer.pAllocator->Reset();

    if (FAILED(result)) {
        R_ERR(R_CHANNEL_WIN32, "Failed to properly reset allocators.");
    }

}


void D3D12Device::initializeBufferResources(U32 buffering)
{
    HRESULT result      = S_OK;
    m_bufferResources.resize(buffering);

    R_DEBUG(R_CHANNEL_D3D12, "Initializing buffer resources.");

    for (U32 i = 0; i < m_bufferResources.size(); ++i) {
        
        result = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
            __uuidof(ID3D12CommandAllocator), (void**)&m_bufferResources[i].pAllocator);

        R_ASSERT(result == S_OK);
    }
}


void D3D12Device::destroyBufferResources()
{
    R_DEBUG(R_CHANNEL_D3D12, "Destroying buffer resources.");

    for (U32 i = 0; i < m_bufferResources.size(); ++i) {
    
        if (m_bufferResources[i].pAllocator) {

            m_bufferResources[i].pAllocator->Release();
            m_bufferResources[i].pAllocator = nullptr;

        }
    
    }

    m_bufferResources.clear();
}


ErrType D3D12Device::createCommandList(D3D12CommandList** ppList, GraphicsQueueTypeFlags flags)
{
    D3D12CommandList* pList = new D3D12CommandList();
    ErrType result = REC_RESULT_OK;

    result = pList->initialize(this, flags);

    if (result != REC_RESULT_OK) {
        R_ERR(R_CHANNEL_D3D12, "Failed to create command list!");
        pList->destroy();

        return result;
    }

    *ppList = pList;

    return result;
}


ErrType D3D12Device::destroyCommandList(D3D12CommandList* pList)
{
    R_ASSERT(pList != NULL);
    
    ErrType result = REC_RESULT_OK;
    result = pList->destroy();

    delete pList;
    
    return result;
    
}



ErrType D3D12Device::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}

// Submits copy of regions from src resource to dst resource. Generally the caller thread will
// be blocked until this function returns, so be sure to use when needed.
ErrType D3D12Device::copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
    CopyBufferRegion* pRegions, U32 numRegions)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse