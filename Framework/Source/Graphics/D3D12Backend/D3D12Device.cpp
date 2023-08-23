//
#include "D3D12Device.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12Allocator.hpp"
#include "D3D12RenderPass.hpp"
#include "D3D12Queue.hpp"
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


GraphicsDevice* D3D12Context::getDevice()
{
    return m_pDevice;
}


ResultCode D3D12Context::setBuffers(U32 bufferCount)
{
    release();
    m_bufferCount = bufferCount;
    initialize();
    m_pDevice->getDescriptorHeapManager()->resizeShaderVisibleHeapInstances(m_bufferCount);
    m_currentBufferIndex = 0;
    return RecluseResult_Ok;
}


ResultCode D3D12Context::wait()
{
    D3D12Queue* pqueue = m_pDevice->getBackbufferQueue();
    D3D12Swapchain* swapchain = m_pDevice->getSwapchain()->castTo<D3D12Swapchain>();
    pqueue->waitForGpu(swapchain->getCurrentFenceValue());
    return RecluseResult_Ok;
}


void D3D12Context::begin()
{
    D3D12Swapchain* swapchain               = m_pDevice->getSwapchain()->castTo<D3D12Swapchain>();

    // TODO: We will want to fix this garbage if statement. If the buffer count is less than the frame count, then
    //       we need to rely on waiting for the buffer fencevalue to finish, instead of the frame fence (resources rely on buffer count.)
    //       If the frame count is less than the buffer count, that is ok, we resort to using the frame fence values instead.
    //       There has to be a cleaner solution, rather than simply using the if statement to determine which is the best strategy.
    if (swapchain->getDesc().desiredFrames <= m_bufferCount)
    {
        swapchain->prepareNextFrame();    
        incrementBufferIndex();
    }
    else
    {
        const U64 currentEventValue = m_bufferResources[getCurrentBufferIndex()].fenceValue;
        incrementBufferIndex();
        swapchain->prepareNextFrameOverride(currentEventValue, m_bufferResources[getCurrentBufferIndex()].fenceValue);
    }
    resetCurrentResources();
    m_pPrimaryCommandList->use(currentBufferIndex());
    m_pPrimaryCommandList->reset();
    m_pPrimaryCommandList->begin();
    prepare();
}


void D3D12Context::end()
{
    // We should always flush any remaining barrier transitions, especially if they involve transiting our back buffer back to present state.
    flushBarrierTransitions();
    D3D12Swapchain* pSwapchain          = m_pDevice->getSwapchain()->castTo<D3D12Swapchain>();

    m_pPrimaryCommandList->end();
    // Fire off our command list!
    pSwapchain->submitPrimaryCommandList(m_pPrimaryCommandList->get());
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


void D3D12Context::transition(GraphicsResource* pResource, ResourceState newState)
{
    if (!pResource->isInResourceState(newState))
    {
        D3D12Resource* d3d12Resource = pResource->castTo<D3D12Resource>();
        D3D12_RESOURCE_BARRIER barrier = d3d12Resource->transition(newState);
        m_barrierTransitions.push_back(barrier);
    }
}


void D3D12Context::flushBarrierTransitions()
{
    if (!m_barrierTransitions.empty())
    {
        ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
        pList->ResourceBarrier(static_cast<UINT>(m_barrierTransitions.size()), m_barrierTransitions.data());
        m_barrierTransitions.clear();
    }
}


void D3D12Context::bindRenderTargets(U32 count, ResourceViewId* ppResources, ResourceViewId pDepthStencil)
{
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    D3D12RenderPass* pRenderPass = RenderPasses::makeRenderPass(m_pDevice, count, ppResources, pDepthStencil);
    R_ASSERT_FORMAT(pRenderPass, "D3D12RenderPass was passed nullptr!");
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRenderPass->getRtvDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pRenderPass->getDsvDescriptor();
    pList->OMSetRenderTargets(count, &rtvHandle, true, dsvHandle.ptr == DescriptorTable::invalidCpuAddress.ptr ? nullptr : &dsvHandle);
    
    m_pRenderPass = pRenderPass;
}


void D3D12Context::clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect)
{
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    R_ASSERT_FORMAT(m_pRenderPass, "No render pass was set for clear! Be sure to call bindRenderTargets() to set up a render pass!");
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pRenderPass->getDsvDescriptor();
    R_ASSERT_FORMAT(dsvHandle.ptr != 0, "Null depth descriptor handle passed to clearDepthStencil()!");

    D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
    if (clearFlags & ClearFlag_Depth)
        flags |= D3D12_CLEAR_FLAG_DEPTH;
    if (clearFlags & ClearFlag_Stencil)
        flags |= D3D12_CLEAR_FLAG_STENCIL;

    D3D12_RECT nativeRect   = { };
    nativeRect.left         = rect.x;
    nativeRect.right        = rect.x + rect.width;
    nativeRect.top          = rect.y;
    nativeRect.bottom       = rect.y + rect.height;
    
    pList->ClearDepthStencilView(dsvHandle, flags, clearDepth, clearStencil, 1, &nativeRect);
}


void D3D12Context::clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect)
{
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    R_ASSERT_FORMAT(m_pRenderPass, "No render pass was set for clear! Be sure to call bindRenderTargets() to set up a render pass!");
    flushBarrierTransitions();
   
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRenderPass->getRtvDescriptor(idx);
    FLOAT clearValue[4];
    D3D12_RECT d3d12Rect = { };

    d3d12Rect.left      = rect.x;
    d3d12Rect.right     = rect.x + rect.width;
    d3d12Rect.top       = rect.y;
    d3d12Rect.bottom    = rect.y + rect.height;

    clearValue[0] = clearColor[0];
    clearValue[1] = clearColor[1];
    clearValue[2] = clearColor[2];
    clearValue[3] = clearColor[3];
    pList->ClearRenderTargetView(rtvHandle, clearValue, 1, &d3d12Rect);
}


void D3D12Context::prepare()
{
    ShaderVisibleDescriptorHeapInstance* shaderVisibleHeap = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(getCurrentBufferIndex());
    shaderVisibleHeap->update(DescriptorHeapUpdateFlag_Reset);

    // We should bind the descriptor heaps at the start of the commandlist.
    ID3D12DescriptorHeap* pHeaps[2] = { nullptr, nullptr };
    pHeaps[0] = shaderVisibleHeap->get(GpuHeapType_CbvSrvUav).getNative();
    pHeaps[1] = shaderVisibleHeap->get(GpuHeapType_Sampler).getNative();
    m_pPrimaryCommandList->bindDescriptorHeaps(pHeaps, 2);
}


ResultCode D3D12Device::initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info)
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

    DescriptorHeapAllocationManager::DescriptorCoreSize descriptorSizes = { };
    m_descHeapManager.initialize(m_device, descriptorSizes, 0);
    m_resourceAllocationManager.initialize(m_device);

    return RecluseResult_Ok;
}


void D3D12Device::destroy()
{
    m_resourceAllocationManager.release();
    RenderPasses::clearAll(this);
    DescriptorViews::clearAll(this);
    m_descHeapManager.release();
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


ResultCode D3D12Device::createSwapchain(D3D12Swapchain** ppSwapchain, const SwapchainCreateDescription& desc)
{
    ResultCode result = RecluseResult_Ok;
    D3D12Swapchain* pSwapchain = new D3D12Swapchain(desc, m_graphicsQueue);

    result = pSwapchain->initialize(this);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create d3d swapchain!");

        pSwapchain->destroy();
        delete pSwapchain;
        return result;
    }

    *ppSwapchain = pSwapchain;

    return result;
}


ResultCode D3D12Device::destroySwapchain(D3D12Swapchain* pSwapchain)
{
    ResultCode result = RecluseResult_Ok;

    pSwapchain->destroy();
    delete pSwapchain;
   
    return result;
}


ResultCode D3D12Device::createCommandQueue(D3D12Queue** ppQueue, GraphicsQueueTypeFlags type)
{
    D3D12Queue* pD3D12Queue = new D3D12Queue(type);
    ResultCode result          = RecluseResult_Ok;

    result = pD3D12Queue->initialize(this);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create d3d12 queue!");

        pD3D12Queue->destroy();
        delete pD3D12Queue;

        return result;    
    }

    *ppQueue = pD3D12Queue;

    return result;
}


ResultCode D3D12Device::destroyCommandQueue(D3D12Queue* pQueue)
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


ResultCode D3D12Device::reserveMemory(const MemoryReserveDescription& desc)
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
        R_ERROR(R_CHANNEL_WIN32, "Failed to properly reset allocators.");
    }    

    ShaderVisibleDescriptorHeapInstance* instance = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(getCurrentBufferIndex());
    instance->update(DescriptorHeapUpdateFlag_Reset);
}


void D3D12Context::initializeBufferResources(U32 buffering)
{
    if (buffering == 0) return;
    HRESULT result      = S_OK;
    m_bufferResources.resize(buffering);

    R_DEBUG(R_CHANNEL_D3D12, "Initializing buffer resources.");
    D3D12Swapchain* swapchain = m_pDevice->getSwapchain()->castTo<D3D12Swapchain>();
    for (U32 i = 0; i < m_bufferResources.size(); ++i) 
    {    
        result = m_pDevice->get()->CreateCommandAllocator
                                (
                                    D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                    __uuidof(ID3D12CommandAllocator), 
                                    (void**)&m_bufferResources[i].pAllocator
                                );

        R_ASSERT(result == S_OK);
        m_bufferResources[i].fenceValue = swapchain->getCurrentCompletedValue();
    }
    m_bufferResources[m_currentBufferIndex].fenceValue = m_pDevice->getBackbufferQueue()->waitForGpu(m_bufferResources[m_currentBufferIndex].fenceValue);
}


void D3D12Context::destroyBufferResources()
{
    R_DEBUG(R_CHANNEL_D3D12, "Destroying buffer resources.");

    for (U32 i = 0; i < m_bufferResources.size(); ++i) 
    {
        if (m_bufferResources[i].pAllocator) 
        {
            m_bufferResources[i].pAllocator->Reset();
            m_bufferResources[i].pAllocator->Release();
            m_bufferResources[i].pAllocator = nullptr;
        }
    }

    m_bufferResources.clear();
}


void D3D12Context::setScissors(U32 numScissors, Rect* pRects)
{
    R_ASSERT(numScissors < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    D3D12_RECT rects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    
    for (U32 i = 0; i < numScissors; ++i)
    {
        rects[i].left   = pRects[i].x;
        rects[i].top    = pRects[i].y;
        rects[i].right  = pRects[i].width;
        rects[i].bottom = pRects[i].height;
    }

    pList->RSSetScissorRects(numScissors, rects);
}


void D3D12Context::setViewports(U32 numViewports, Viewport* pViewports)
{
    R_ASSERT(numViewports < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    D3D12_VIEWPORT viewports[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    for (U32 i = 0; i < numViewports; ++i)
    {
        viewports[i].TopLeftX   = pViewports[i].x;
        viewports[i].TopLeftY   = pViewports[i].y;
        viewports[i].Width      = pViewports[i].width;
        viewports[i].Height     = pViewports[i].height;
        viewports[i].MaxDepth   = pViewports[i].maxDepth;
        viewports[i].MinDepth   = pViewports[i].minDepth;
    }
    
    pList->RSSetViewports(numViewports, viewports);
}


ResultCode D3D12Context::createCommandList(D3D12PrimaryCommandList** ppList, GraphicsQueueTypeFlags flags)
{
    D3D12PrimaryCommandList* pList = new D3D12PrimaryCommandList();
    ResultCode result = RecluseResult_Ok;

    result = pList->initialize(this, flags);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create command list!");
        pList->destroy();

        return result;
    }

    *ppList = pList;

    return result;
}


ResultCode D3D12Context::destroyCommandList(D3D12PrimaryCommandList* pList)
{
    R_ASSERT(pList != NULL);
    
    ResultCode result = RecluseResult_Ok;
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
        R_ERROR(R_CHANNEL_D3D12, "Failed to query for proper format support on device!");
    }

    return formatSupport;
}


ResultCode D3D12Device::loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition)
{
    return RecluseResult_NoImpl;
}


ResultCode D3D12Device::unloadShaderProgram(ShaderProgramId program)
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


GraphicsContext* D3D12Device::createContext()
{
    D3D12Context* pContext = new D3D12Context(this, 0);
    return pContext;
}


ResultCode D3D12Device::releaseContext(GraphicsContext* pContext)
{
    R_ASSERT(pContext);
    D3D12Context* d3d12Context = pContext->castTo<D3D12Context>();
    d3d12Context->release();
    delete d3d12Context;
    return RecluseResult_Ok;
}


ResultCode D3D12Device::createResource(GraphicsResource** ppResource, const GraphicsResourceDescription& description, ResourceState initState)
{
    D3D12Resource* pResource = makeResource(this, description, initState);
    if (!pResource)
    {
        return RecluseResult_Failed;
    }
    *ppResource = pResource;
    return RecluseResult_Ok;
}


ResultCode D3D12Device::destroyResource(GraphicsResource* pResource)
{
    R_ASSERT(pResource != NULL);
    D3D12Resource* pD3D12Resource = pResource->castTo<D3D12Resource>();
    return releaseResource(pD3D12Resource);
}
} // Recluse