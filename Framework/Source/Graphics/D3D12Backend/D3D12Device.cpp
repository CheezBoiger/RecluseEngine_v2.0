//
#include "D3D12Device.hpp"
#include "D3D12Instance.hpp"
#include "D3D12Adapter.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Swapchain.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12Allocator.hpp"
#include "D3D12RenderPass.hpp"
#include "D3D12Queue.hpp"
#include "D3D12ShaderCache.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace D3D12 {

void D3D12Context::initialize()
{
    initializeBufferResources(m_bufferCount);
    createCommandList(&m_pPrimaryCommandList, QUEUE_TYPE_PRESENT | QUEUE_TYPE_GRAPHICS);
    
    D3D12ResourceAllocationManager* manager = m_pDevice->resourceAllocationManager();
    D3D12ResourceAllocationManager::Update update = { };
    update.flags = D3D12ResourceAllocationManager::UpdateFlag_SetFrameIndex | D3D12ResourceAllocationManager::UpdateFlag_ResizeGarbage;
    update.frameIndex = getCurrentFrameIndex();
    update.frameSize = m_bufferCount;
    manager->update(update);    
    // Preallocate to 256 possible barriers in a sitting.
    m_barrierTransitions.reserve(256);
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


ResultCode D3D12Context::setFrames(U32 bufferCount)
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
    D3D12Queue* pqueue = m_queue;
    pqueue->waitForGpu(getContextFrame(getCurrentFrameIndex()).fenceValue);
    return RecluseResult_Ok;
}


void D3D12Context::begin()
{
    ID3D12Fence* fence = m_queue->getFence();
    HANDLE e = m_queue->getEvent();
    const U64 previousFenceValue = getContextFrame(getCurrentFrameIndex()).fenceValue;

    incrementBufferIndex();

    const U64 currentFrameValue = getContextFrame(getCurrentFrameIndex()).fenceValue;
    m_queue->get()->Signal(fence, previousFenceValue);
    const U64 completedValue = fence->GetCompletedValue();
    if (completedValue < currentFrameValue)
    {
        fence->SetEventOnCompletion(currentFrameValue, e);
        WaitForSingleObject(e, INFINITE);
    }
    // Reset the current fence value with the previous + 1. This will ensure we 
    // are ahead of the fence value for the next frame work.
    setNewFenceValue(getCurrentFrameIndex(), previousFenceValue + 1);

    resetCurrentResources();
    m_pPrimaryCommandList->use(currentBufferIndex());
    m_pPrimaryCommandList->reset();
    m_pPrimaryCommandList->begin();
    prepare();
}


ResultCode D3D12Context::submitPrimaryCommandList(ID3D12GraphicsCommandList* pCommandList)
{
    ID3D12CommandQueue* pPresentationQueue = m_queue->get();
    ID3D12CommandList* pLists[] = { pCommandList };

    R_ASSERT(pPresentationQueue != NULL);
    R_ASSERT(pCommandList != NULL);

    pPresentationQueue->ExecuteCommandLists(1, pLists);
    return RecluseResult_Ok;
}


void D3D12Context::end()
{
    // We should always flush any remaining barrier transitions, especially if they involve transiting our back buffer back to present state.
    flushBarrierTransitions();
    m_pPrimaryCommandList->end();
    // Fire off our command list!
    submitPrimaryCommandList(m_pPrimaryCommandList->get());
    RenderPasses::sweep(m_pDevice);
    Pipelines::checkPipelines(m_pDevice);
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


void D3D12Context::transition(GraphicsResource* pResource, ResourceState newState, U16 baseMip, U16 mipCount, U16 baseLayer, U16 layerCount)
{
    if (!pResource->isInResourceState(newState))
    {
        D3D12Resource* d3d12Resource = pResource->castTo<D3D12Resource>();

        if (mipCount == 0 && layerCount == 0)
        {
            D3D12_RESOURCE_BARRIER barrier = d3d12Resource->transition(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, newState);
            m_barrierTransitions.push_back(barrier);
        }
        else
        {
            D3D12_RESOURCE_DESC resourceDesc = pResource->castTo<D3D12Resource>()->get()->GetDesc();
            U32 mipLevels = resourceDesc.MipLevels;
            U32 maxLayers = resourceDesc.DepthOrArraySize;
            if (mipCount == 0) mipCount = mipLevels;
            if (layerCount == 0) layerCount = maxLayers;

            for (U32 mip = baseMip; mip < mipCount; ++mip)
            {
                for (U32 layer = baseLayer; layer < layerCount; ++layer)
                {
                    U32 subresource = mip + (layer * mipLevels) + (0 * mipLevels * maxLayers);
                    D3D12_RESOURCE_BARRIER barrier = d3d12Resource->transition(subresource, newState);
                    m_barrierTransitions.push_back(barrier);
                }
            }
        }
        d3d12Resource->finalizeTransition(newState);
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
    if (currentState().m_currentRenderPass != pRenderPass)
    {
        R_ASSERT_FORMAT(pRenderPass, "D3D12RenderPass was passed nullptr!");
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRenderPass->getRtvDescriptor();
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pRenderPass->getDsvDescriptor();
        pList->OMSetRenderTargets(count, &rtvHandle, true, dsvHandle.ptr == DescriptorTable::invalidCpuAddress.ptr ? nullptr : &dsvHandle);
    
        currentState().m_currentRenderPass = pRenderPass;
        currentState().m_pipelineStateObject.graphics.numRenderTargets = count;

        Bool shouldConfigPipeline = false;
        for (U32 i = 0; i < count; ++i)
        {
            DXGI_FORMAT format = pRenderPass->getRtvFormat(i);
            if (format != currentState().m_pipelineStateObject.graphics.rtvFormats[i])
                shouldConfigPipeline = true;
            currentState().m_pipelineStateObject.graphics.rtvFormats[i] = format;
        }

        if (dsvHandle.ptr != DescriptorTable::invalidCpuAddress.ptr)
        {
            DXGI_FORMAT format = pRenderPass->getDsvFormat();
            if (format != currentState().m_pipelineStateObject.graphics.dsvFormat)
            {
                shouldConfigPipeline = true;
            }
            currentState().m_pipelineStateObject.graphics.dsvFormat = format;
        }

        if (shouldConfigPipeline)
            currentState().setDirty(ContextDirty_Pipeline);
    }
}


void D3D12Context::clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect)
{
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    R_ASSERT_FORMAT(currentState().m_currentRenderPass, "No render pass was set for clear! Be sure to call bindRenderTargets() to set up a render pass!");
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = currentState().m_currentRenderPass->getDsvDescriptor();
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
    R_ASSERT_FORMAT(currentState().m_currentRenderPass, "No render pass was set for clear! Be sure to call bindRenderTargets() to set up a render pass!");
    flushBarrierTransitions();
   
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = currentState().m_currentRenderPass->getRtvDescriptor(idx);
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
    D3D12ResourceAllocationManager* manager = m_pDevice->resourceAllocationManager();
    D3D12ResourceAllocationManager::Update update = { };
    update.flags = D3D12ResourceAllocationManager::UpdateFlag_SetFrameIndex | D3D12ResourceAllocationManager::UpdateFlag_Update;
    update.frameIndex = getCurrentFrameIndex();
    manager->update(update);    

    ShaderVisibleDescriptorHeapInstance* shaderVisibleHeap = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(getCurrentFrameIndex());
    shaderVisibleHeap->update(DescriptorHeapUpdateFlag_Reset);

    // Update the age tick for renderpasses.
    RenderPasses::update();
    Pipelines::updateT(m_pDevice);

    // We should bind the descriptor heaps at the start of the commandlist.
    ID3D12DescriptorHeap* pHeaps[2] = { nullptr, nullptr };
    pHeaps[0] = shaderVisibleHeap->get(GpuHeapType_CbvSrvUav).getNative();
    pHeaps[1] = shaderVisibleHeap->get(GpuHeapType_Sampler).getNative();
    m_pPrimaryCommandList->bindDescriptorHeaps(pHeaps, 2);
}


ShaderVisibleDescriptorTable D3D12Context::uploadToShaderVisible(CpuDescriptorTable table, GpuHeapType shaderVisibleType)
{
    DescriptorHeapAllocationManager* manager        = m_pDevice->getDescriptorHeapManager();
    ShaderVisibleDescriptorHeapInstance* instance   = manager->getShaderVisibleInstance(getCurrentFrameIndex());
    return instance->upload(m_pDevice->get(), shaderVisibleType, table);
}


ResultCode D3D12Device::initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating D3D12 device...");

    IDXGIAdapter* pAdapter  = adapter->get();
    HRESULT result          = S_OK;

    DXGI_ADAPTER_DESC adapterDescription;
    pAdapter->GetDesc(&adapterDescription);

    result = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&m_device);

    if (result != S_OK) 
    {
        return RecluseResult_Failed;
    }

    m_pAdapter = adapter;

    createCommandQueues();

    DescriptorHeapAllocationManager::DescriptorCoreSize descriptorSizes = { };
    m_descHeapManager.initialize(m_device, descriptorSizes, 0);
    m_resourceAllocationManager.initialize(m_device);

    if (adapter->getInstance()->isLayerFeatureEnabled(LayerFeatureFlag_DebugValidation | LayerFeatureFlag_GpuDebugValidation))
    {
        m_debugCookie = adapter->getInstance()->registerDebugMessageCallback(m_device);
    }

    R_DEBUG(R_CHANNEL_D3D12, "Successfully created D3D12 device!");
    return RecluseResult_Ok;
}


void D3D12Device::createCommandQueues()
{
    D3D12Queue queue = createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_queues.insert(std::make_pair(D3D12_COMMAND_LIST_TYPE_DIRECT, queue));
}


void D3D12Device::destroyCommandQueues()
{
    for (auto& iter : m_queues)
    {
        destroyCommandQueue(iter.second);
    }
    m_queues.clear();
}


void D3D12Device::destroy()
{
    if (m_debugCookie > 0)
    {
        m_pAdapter->getInstance()->unregisterDebugMessageCallback(m_device, m_debugCookie);
    }

    m_resourceAllocationManager.release();
    RenderPasses::clearAll(this);
    Pipelines::VertexInputs::unloadAll();
    Pipelines::cleanUpPipelines();
    unloadAllShaderPrograms();
    Pipelines::cleanUpRootSigs();
    DescriptorViews::clearAll(this);
    m_descHeapManager.release();

    destroyCommandQueues();

    if (m_device) 
    {
        R_DEBUG(R_CHANNEL_D3D12, "Destroying D3D12 device...");

        m_device->Release();
    }
}


GraphicsSwapchain* D3D12Device::createSwapchain(const SwapchainCreateDescription& desc, void* windowHandle)
{
    ResultCode result = RecluseResult_Ok;
    D3D12Swapchain* pSwapchain = new D3D12Swapchain(desc, getQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

    result = pSwapchain->initialize(this, (HWND)windowHandle);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create d3d swapchain!");

        pSwapchain->destroy();
        delete pSwapchain;
    }

    return pSwapchain;
}


ResultCode D3D12Device::destroySwapchain(GraphicsSwapchain* pSwapchain)
{
    ResultCode result = RecluseResult_Ok;
    D3D12Swapchain* d3dSwapchain = pSwapchain->castTo<D3D12Swapchain>();
    d3dSwapchain->destroy();
    delete d3dSwapchain;
   
    return result;
}


D3D12Queue D3D12Device::createCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
    D3D12Queue pD3D12Queue = { };
    ResultCode result          = RecluseResult_Ok;

    result = pD3D12Queue.initialize(get(), type);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create d3d12 queue!");
        pD3D12Queue.destroy();    
    }
    return pD3D12Queue;
}


ResultCode D3D12Device::destroyCommandQueue(D3D12Queue& queue)
{
    queue.destroy();
    return RecluseResult_Ok;
}


ResultCode D3D12Device::reserveMemory(const MemoryReserveDescription& desc)
{
    return m_resourceAllocationManager.reserveMemory(desc);
}


void D3D12Context::resetCurrentResources()
{
    HRESULT result = S_OK;
    ContextFrame& buffer = m_contextFrames[m_currentBufferIndex];
        
    result = buffer.pAllocator->Reset();

    if (FAILED(result)) 
    {
        R_ERROR(R_CHANNEL_WIN32, "Failed to properly reset allocators.");
    }

    // Must always start with one main context state.
    m_contextStates.clear();
    m_contextStates.push_back({ });
    clearResourceBinds();

    Pipelines::resetTableHeaps(m_pDevice);
    RenderPasses::clearRenderPassCache();
    ShaderVisibleDescriptorHeapInstance* instance = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(getCurrentFrameIndex());
    instance->update(DescriptorHeapUpdateFlag_Reset);
}


void D3D12Context::initializeBufferResources(U32 buffering)
{
    if (buffering == 0) return;
    HRESULT result      = S_OK;
    m_contextFrames.resize(buffering);

    R_DEBUG(R_CHANNEL_D3D12, "Initializing buffer resources.");
    for (U32 i = 0; i < m_contextFrames.size(); ++i) 
    {    
        result = m_pDevice->get()->CreateCommandAllocator
                                (
                                    D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                    __uuidof(ID3D12CommandAllocator), 
                                    (void**)&m_contextFrames[i].pAllocator
                                );

        R_ASSERT(result == S_OK);
        m_contextFrames[i].fenceValue = m_queue->getFence()->GetCompletedValue();
    }
    m_contextFrames[m_currentBufferIndex].fenceValue = m_queue->waitForGpu(m_contextFrames[m_currentBufferIndex].fenceValue);
}


void D3D12Context::destroyBufferResources()
{
    R_DEBUG(R_CHANNEL_D3D12, "Destroying buffer resources.");

    for (U32 i = 0; i < m_contextFrames.size(); ++i) 
    {
        if (m_contextFrames[i].pAllocator) 
        {
            m_contextFrames[i].pAllocator->Reset();
            m_contextFrames[i].pAllocator->Release();
            m_contextFrames[i].pAllocator = nullptr;
        }
    }

    m_contextFrames.clear();
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
    flushBarrierTransitions();
    D3D12Queue::generateCopyResourceCommand(m_pPrimaryCommandList->get(), dst->castTo<D3D12Resource>(), src->castTo<D3D12Resource>()); 
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
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    ID3D12Resource* dstRes = dst->castTo<D3D12Resource>()->get();
    ID3D12Resource* srcRes = src->castTo<D3D12Resource>()->get();
    for (U32 i = 0; i < numRegions; ++i)
    {
        const CopyBufferRegion& region = pRegions[i];
        pList->CopyBufferRegion(dstRes, region.dstOffsetBytes, srcRes, region.srcOffsetBytes, region.szBytes);
    }
}


ResultCode D3D12Device::createSampler(GraphicsSampler** sampler, const SamplerDescription& desc)
{
    D3D12Sampler* d3d12Sampler = DescriptorViews::makeSampler(this, desc);
    *sampler = d3d12Sampler;
    return RecluseResult_Ok;
}


ResultCode D3D12Device::destroySampler(GraphicsSampler* sampler)
{
    D3D12Sampler* d3d12Sampler = sampler->castTo<D3D12Sampler>();
    if (!d3d12Sampler)
    {
        return RecluseResult_NullPtrExcept;
    }
    DescriptorViews::destroySampler(this, d3d12Sampler);
    return RecluseResult_Ok;
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


ResultCode D3D12Device::loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition)
{
    if (D3D::Cache::isProgramCached(program, permutation))
    {
        return RecluseResult_NeedsUpdate;
    }
    return D3D::Cache::loadNativeShaderProgramPermutation(program, permutation, definition);
}


ResultCode D3D12Device::unloadShaderProgram(ShaderProgramId program)
{
    if (!D3D::Cache::isProgramCached(program))
    {
        return RecluseResult_NotFound;
    }
    return D3D::Cache::unloadPrograms(program);
}


void D3D12Device::unloadAllShaderPrograms()
{
    D3D::Cache::unloadAll();
}


Bool D3D12Device::makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout)
{
    ResultCode result = Pipelines::VertexInputs::make(id, layout);
    return (result == RecluseResult_Ok) || (result == RecluseResult_AlreadyExists);
}


Bool D3D12Device::destroyVertexLayout(VertexInputLayoutId id)
{
    return Pipelines::VertexInputs::unload(id);
}


GraphicsContext* D3D12Device::createContext()
{
    D3D12Context* pContext = new D3D12Context(this, 0, getQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
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


ResultCode D3D12Device::destroyResource(GraphicsResource* pResource, Bool immediate)
{
    R_ASSERT(pResource != NULL);
    D3D12Resource* pD3D12Resource = pResource->castTo<D3D12Resource>();
    return releaseResource(pD3D12Resource, immediate);
}


void D3D12Device::copyResource(GraphicsResource* dst, GraphicsResource* src)
{
    getQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->copyResource(dst->castTo<D3D12Resource>(), src->castTo<D3D12Resource>());
}


void D3D12Device::copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, const CopyBufferRegion* regions, U32 numRegions)
{
    getQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->copyBufferRegions(dst->castTo<D3D12Resource>(), src->castTo<D3D12Resource>(), regions, numRegions);
}
} // D3D12
} // Recluse