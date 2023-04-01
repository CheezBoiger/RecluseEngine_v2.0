//
#include "D3D12Device.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Resource.hpp"
#include "D3D12Allocator.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


void D3D12Context::initialize()
{
    initializeBufferResources(m_bufferCount);
    createCommandList(&m_pPrimaryCommandList, QUEUE_TYPE_PRESENT | QUEUE_TYPE_GRAPHICS);
}


void D3D12Context::release()
{
    if (m_pPrimaryCommandList)
    {
        destroyCommandList(m_pPrimaryCommandList);
        m_pPrimaryCommandList = nullptr;
    }

    destroyBufferResources();
}


void D3D12Context::begin()
{

    incrementBufferIndex();

    BufferResources* pBufferResources   = getCurrentBufferResource();
    ID3D12Fence* pFence                 = pBufferResources->pFence;
    HANDLE eventHandle                  = pBufferResources->pEvent;
    U64 currentValue                    = pBufferResources->fenceValue;

    if (pFence->GetCompletedValue() < currentValue)
    {
        pFence->SetEventOnCompletion(currentValue, eventHandle);
        WaitForSingleObjectEx(eventHandle, INFINITE, false);
    }

    resetCurrentResources();

    m_pPrimaryCommandList->reset();
}


void D3D12Context::end()
{
    m_pPrimaryCommandList->end();
}


void D3D12Context::pushState(ContextFlags flags)
{
    if (flags & ContextFlag_InheritPipelineState)
    {
        const ContextState& contextState = m_contextStates.back(); 
        m_contextStates.push_back(contextState);
    }
    else
    {
        m_contextStates.push_back({ });
    }
}


void D3D12Context::popState()
{
    R_ASSERT_FORMAT(!m_contextStates.empty(), "Context states must not be empty!");
    m_contextStates.pop_back();
}


ErrType D3D12Device::initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating D3D12 device...");

    IDXGIAdapter* pAdapter  = adapter->get();
    HRESULT result          = S_OK;

    result = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&m_device);


    if (result != S_OK) 
    {
        return RecluseResult_Failed;
    }

    m_pAdapter = adapter;

    if (info.winHandle) 
    {
        m_windowHandle = (HWND)info.winHandle;
        createCommandQueue(&m_graphicsQueue, QUEUE_TYPE_PRESENT);
    }

    R_DEBUG(R_CHANNEL_D3D12, "Successfully created D3D12 device!");

    if (m_windowHandle) 
    {
        createSwapchain(&m_swapchain, info.swapchainDescription);
    }

    return RecluseResult_Ok;
}


void D3D12Device::destroy()
{
    m_context->release();
    delete m_context;
    m_context = nullptr;

    if (m_swapchain) 
    {
        destroySwapchain(m_swapchain);
        m_swapchain = nullptr;
    }

    if (m_graphicsQueue) 
    {
        destroyCommandQueue(m_graphicsQueue);
        m_graphicsQueue = nullptr;
    }

    if (m_device) 
    {
        R_DEBUG(R_CHANNEL_D3D12, "Destroying D3D12 device...");

        m_device->Release();
    }
}


ErrType D3D12Device::createSwapchain(D3D12Swapchain** ppSwapchain, const SwapchainCreateDescription& desc)
{
    ErrType result = RecluseResult_Ok;
    D3D12Swapchain* pSwapchain = new D3D12Swapchain(desc, m_graphicsQueue);

    result = pSwapchain->initialize(this);

    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to create d3d swapchain!");

        pSwapchain->destroy();
        delete pSwapchain;
        return result;
    }

    *ppSwapchain = pSwapchain;

    return result;
}


ErrType D3D12Device::destroySwapchain(D3D12Swapchain* pSwapchain)
{
    ErrType result = RecluseResult_Ok;

    pSwapchain->destroy();
    delete pSwapchain;
   
    return result;
}


ErrType D3D12Device::createCommandQueue(D3D12Queue** ppQueue, GraphicsQueueTypeFlags type)
{
    D3D12Queue* pD3D12Queue = new D3D12Queue(type);
    ErrType result          = RecluseResult_Ok;

    result = pD3D12Queue->initialize(this);

    if (result != RecluseResult_Ok) 
    {
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
    if (!pQueue) 
    {
        return RecluseResult_NullPtrExcept;
    }

    D3D12Queue* pD3D12Queue = pQueue;

    pD3D12Queue->destroy();

    delete pD3D12Queue;

    return RecluseResult_Ok;
}


void D3D12Device::allocateMemoryPool(D3D12MemoryPool* pPool, ResourceMemoryUsage memUsage)
{

}


ErrType D3D12Device::reserveMemory(const MemoryReserveDescription& desc)
{
    return m_resourceAllocationManager.reserveMemory(desc);
}


void D3D12Context::resetCurrentResources()
{
    HRESULT result = S_OK;
    BufferResources& buffer = m_bufferResources[m_currentBufferIndex];
        
    result = buffer.pAllocator->Reset();

    if (FAILED(result)) 
    {
        R_ERR(R_CHANNEL_WIN32, "Failed to properly reset allocators.");
    }

}


void D3D12Context::initializeBufferResources(U32 buffering)
{
    HRESULT result      = S_OK;
    m_bufferResources.resize(buffering);

    R_DEBUG(R_CHANNEL_D3D12, "Initializing buffer resources.");

    for (U32 i = 0; i < m_bufferResources.size(); ++i) 
    {    
        result = m_pDevice->get()->CreateCommandAllocator
                                (
                                    D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                    __uuidof(ID3D12CommandAllocator), 
                                    (void**)&m_bufferResources[i].pAllocator
                                );

        R_ASSERT(result == S_OK);

        m_bufferResources[i].fenceValue = 0;
        m_bufferResources[i].pEvent     = CreateEvent(NULL, FALSE, FALSE, NULL);

        result = m_pDevice->get()->CreateFence
                                (
                                    0, 
                                    D3D12_FENCE_FLAG_NONE, 
                                    __uuidof(ID3D12Fence), 
                                    (void**)&m_bufferResources[i].pFence
                                );

        if (FAILED(result)) 
        {
            R_ERR(R_CHANNEL_D3D12, "Failed to initialize a fence for frame resource: %d", i);
        }
    }
}


void D3D12Context::destroyBufferResources()
{
    R_DEBUG(R_CHANNEL_D3D12, "Destroying buffer resources.");

    for (U32 i = 0; i < m_bufferResources.size(); ++i) 
    {
        if (m_bufferResources[i].pAllocator) 
        {
            m_bufferResources[i].pAllocator->Release();
            m_bufferResources[i].pAllocator = nullptr;

            CloseHandle(m_bufferResources[i].pEvent);
            m_bufferResources[i].pEvent = NULL;

            m_bufferResources[i].pFence->Release();
            m_bufferResources[i].pFence = nullptr;
        }
    }

    m_bufferResources.clear();
}


ErrType D3D12Context::createCommandList(D3D12PrimaryCommandList** ppList, GraphicsQueueTypeFlags flags)
{
    D3D12PrimaryCommandList* pList = new D3D12PrimaryCommandList();
    ErrType result = RecluseResult_Ok;

    result = pList->initialize(this, flags);

    if (result != RecluseResult_Ok) 
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to create command list!");
        pList->destroy();

        return result;
    }

    *ppList = pList;

    return result;
}


ErrType D3D12Context::destroyCommandList(D3D12PrimaryCommandList* pList)
{
    R_ASSERT(pList != NULL);
    
    ErrType result = RecluseResult_Ok;
    result = pList->destroy();

    delete pList;
    
    return result;
    
}


void D3D12Context::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    R_NO_IMPL();
}


// Submits copy of regions from src resource to dst resource. Generally the caller thread will
// be blocked until this function returns, so be sure to use when needed.
void D3D12Context::copyBufferRegions
    (
        GraphicsResource* dst, 
        GraphicsResource* src, 
        const CopyBufferRegion* pRegions, 
        U32 numRegions
    )
{
    R_NO_IMPL();
}


GraphicsSwapchain* D3D12Device::getSwapchain()
{
    return m_swapchain;
}


void D3D12Device::createSampler(const D3D12_SAMPLER_DESC& desc)
{
    R_NO_IMPL();
}


void D3D12Device::createRenderTargetView(D3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    m_device->CreateRenderTargetView(pResource->get(), &desc, destDescriptor);
}


void D3D12Device::createDepthStencilView(D3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    m_device->CreateDepthStencilView(pResource->get(), &desc, destDescriptor);
}


void D3D12Device::createShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
    R_NO_IMPL();
}


void D3D12Device::createUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    R_NO_IMPL();
}


D3D12_FEATURE_DATA_FORMAT_SUPPORT D3D12Device::checkFormatSupport(ResourceFormat format)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { };

    formatSupport.Format = Dxgi::getNativeFormat(format);

    if 
        (
            FAILED
                (
                    m_device->CheckFeatureSupport
                                    (
                                        D3D12_FEATURE_FORMAT_SUPPORT, 
                                        &formatSupport, 
                                        sizeof(formatSupport)
                                    )
                )
        )
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to query for proper format support on device!");
    }

    return formatSupport;
}


ErrType D3D12Device::loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition)
{
    return RecluseResult_NoImpl;
}


ErrType D3D12Device::unloadShaderProgram(ShaderProgramId program)
{
    return RecluseResult_NoImpl;
}


void D3D12Device::unloadAllShaderPrograms()
{
    R_NO_IMPL();
}


Bool D3D12Device::makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout)
{
    R_NO_IMPL();
    return false;
}


Bool D3D12Device::destroyVertexLayout(VertexInputLayoutId id)
{
    R_NO_IMPL();
    return false;
}
} // Recluse