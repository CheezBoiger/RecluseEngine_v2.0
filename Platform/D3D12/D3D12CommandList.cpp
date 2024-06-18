//
#include "D3D12CommandList.hpp"
#include "D3D12Device.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"
#include "D3D12ShaderCache.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

namespace Recluse {
namespace D3D12 {

ResultCode D3D12PrimaryCommandList::initialize(D3D12Context* pDeviceContext, GraphicsQueueTypeFlags flags)
{
    flags;

    HRESULT result                                      = S_OK;
    const std::vector<ContextFrame>& bufferResources = pDeviceContext->getContextFrames();
    ID3D12Device* device                                = staticCast<D3D12Device*>(pDeviceContext->getDevice())->get();

    m_allocators.resize(bufferResources.size());
    m_graphicsCommandLists.resize(m_allocators.size());

    for (U32 i = 0; i < m_allocators.size(); ++i) 
    {
        m_allocators[i] = bufferResources[i].pAllocator;

        result = device->CreateCommandList
                            (
                                0, 
                                D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                m_allocators[i], 
                                nullptr, 
                                __uuidof(ID3D12GraphicsCommandList4), 
                                (void**)&m_graphicsCommandLists[i]
                            );

        if (FAILED(result)) 
        {    
            R_ERROR(R_CHANNEL_D3D12, "Failed to create d3d12 graphics command list!");
            
            return destroy();
        }

        // Close first.
        m_graphicsCommandLists[i]->Close();
    }

    D3D12_COMMAND_SIGNATURE_DESC signatureDescription = { };
    D3D12_INDIRECT_ARGUMENT_DESC indirectArgsDescription = { };
    signatureDescription.NumArgumentDescs = 1;
    signatureDescription.pArgumentDescs = &indirectArgsDescription;

    indirectArgsDescription.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    signatureDescription.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
    ID3D12CommandSignature* signature = nullptr;
    result = device->CreateCommandSignature(&signatureDescription, nullptr, __uuidof(ID3D12CommandSignature), (void**)&signature);
    R_ASSERT(SUCCEEDED(result));
    m_signatureMap[D3D12_INDIRECT_ARGUMENT_TYPE_DRAW] = signature;
    
    indirectArgsDescription.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    signatureDescription.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
    result = device->CreateCommandSignature(&signatureDescription, nullptr, __uuidof(ID3D12CommandSignature), (void**)&signature);
    R_ASSERT(SUCCEEDED(result));
    m_signatureMap[D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED] = signature;

    indirectArgsDescription.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
    signatureDescription.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
    result = device->CreateCommandSignature(&signatureDescription, nullptr, __uuidof(ID3D12CommandSignature), (void**)&signature);
    R_ASSERT(SUCCEEDED(result));
    m_signatureMap[D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH] = signature;
    return RecluseResult_Ok;
}


ResultCode D3D12PrimaryCommandList::destroy()
{
    for (U32 i = 0; i < m_graphicsCommandLists.size(); ++i)
    {
        m_graphicsCommandLists[i]->Release();
    }
    for (auto& iter : m_signatureMap)
    {
        iter.second->Release();
    }
    m_signatureMap.clear();
    return RecluseResult_Ok;
}


void D3D12PrimaryCommandList::begin()
{
    m_status = CommandList_Recording;
}


void D3D12PrimaryCommandList::reset()
{
    R_ASSERT(!m_graphicsCommandLists.empty());
    R_ASSERT(!m_allocators.empty());
    m_currentCmdList->Reset(m_currentAllocator, nullptr);
    m_status = CommandList_Reset;
}


void D3D12PrimaryCommandList::end()
{
    m_currentCmdList->Close();
}


void D3D12PrimaryCommandList::use(U32 bufferIdx)
{
    R_ASSERT_FORMAT(bufferIdx < m_graphicsCommandLists.size(), "Commandlist use() requested idx=%d, but max is %d!", bufferIdx, static_cast<U32>(m_graphicsCommandLists.size()));
    m_currentCmdList = m_graphicsCommandLists[bufferIdx];
    m_currentAllocator = m_allocators[bufferIdx];
}


void D3D12PrimaryCommandList::bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* offsets)
{
    // TO BE DEPRECATED.
    R_NO_IMPL();
}


void D3D12PrimaryCommandList::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type)
{
    // TO BE DEPRECATED.
    R_NO_IMPL();
}


void D3D12PrimaryCommandList::bindDescriptorHeaps(ID3D12DescriptorHeap* const* pHeaps, U32 numHeaps)
{
    get()->SetDescriptorHeaps(numHeaps, pHeaps);
}


void D3D12Context::drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance)
{
    ContextState& state = currentState();
    ID3D12GraphicsCommandList* list = m_pPrimaryCommandList->get();
    flushBarrierTransitions();
    bindRootSignature(list, state);
    bindPipeline(list, state);
    bindCurrentResources();
    internalBindVertexBuffersAndIndexBuffer(list, state);
    currentState().setClean();
    list->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}


void D3D12Context::drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance)
{
    ContextState& state = currentState();
    ID3D12GraphicsCommandList* list = m_pPrimaryCommandList->get();
    flushBarrierTransitions();
    bindRootSignature(list, state);
    bindPipeline(list, state);
    bindCurrentResources();
    internalBindVertexBuffersAndIndexBuffer(list, state);
    currentState().setClean();
    list->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}


void D3D12Context::dispatch(U32 x, U32 y, U32 z)
{
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    ContextState& state = currentState();
    flushBarrierTransitions();
    bindRootSignature(pList, state);
    bindPipeline(pList, state);
    bindCurrentResources();
    state.setClean();
    pList->Dispatch(x, y, z);
}


D3D12Context::ContextState& D3D12Context::D3D12ShaderProgramBinder::currentState()
{
    return m_pContext->currentState();
}


IShaderProgramBinder& D3D12Context::D3D12ShaderProgramBinder::bindShaderResource(ShaderStageFlags type, U32 slot, ResourceViewId view)
{
    D3D12Context* context = m_pContext;
    DescriptorHeapAllocationManager* manager = context->getNativeDevice()->getDescriptorHeapManager();
    ContextState& current = currentState();
    D3D12GraphicsResourceView* pView = DescriptorViews::findResourceView(view);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = manager->nullSrvDescriptor();
    if (pView) handle = pView->getCpuDescriptor();

    current.m_srvs[slot] = handle;
    if (!cachedReflection)
        current.m_rootSigLayout.srvCount = Math::maximum(current.m_rootSigLayout.srvCount, static_cast<U16>(slot+1));
    current.m_resourceTable.srvs = current.m_srvs.data();
    current.setDirty(ContextDirty_Descriptors);
    return (*this);
}


IShaderProgramBinder& D3D12Context::D3D12ShaderProgramBinder::bindConstantBuffer(ShaderStageFlags type, U32 slot, GraphicsResource* pResource, U32 offsetBytes, U32 sizeBytes, void* data)
{
    D3D12Context* context = m_pContext;
    DescriptorHeapAllocationManager* manager = context->getNativeDevice()->getDescriptorHeapManager();
    ContextState& current = currentState();
    D3D12_CPU_DESCRIPTOR_HANDLE cbv = manager->nullCbvDescriptor();
    if (pResource)
    {
        D3D12Resource* pNativeResource = pResource->castTo<D3D12Resource>();
        cbv = pNativeResource->asCbv(offsetBytes, sizeBytes);

        // Copy any data we may want to have in this constant buffer bind. It is actually similar to the vulkan version, so we might
        // want to make this agnostic.
        if (data)
        {
            void* pData = nullptr;
            MapRange readRange;
            readRange.offsetBytes = offsetBytes;
            readRange.sizeBytes = sizeBytes;
            ResultCode result = pNativeResource->map(&pData, &readRange);
            if (result == RecluseResult_Ok)
            {
                memcpy(pData, data, sizeBytes);
                pNativeResource->unmap(&readRange);
            }
        }
    }
    current.m_cbvs[slot] = cbv;
    
    if (!cachedReflection)
        current.m_rootSigLayout.cbvCount = Math::maximum(current.m_rootSigLayout.cbvCount, static_cast<U16>(slot+1));
    
    current.m_resourceTable.cbvs = current.m_cbvs.data();
    current.setDirty(ContextDirty_Descriptors);
    return (*this);
}


IShaderProgramBinder& D3D12Context::D3D12ShaderProgramBinder::bindUnorderedAccessView(ShaderStageFlags type, U32 slot, ResourceViewId view)
{
    D3D12Context* context = m_pContext;
    DescriptorHeapAllocationManager* manager = context->getNativeDevice()->getDescriptorHeapManager();
    ContextState& current = currentState();
    D3D12GraphicsResourceView* pView = DescriptorViews::findResourceView(view);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = manager->nullUavDescriptor(); 
    if (pView) handle = pView->getCpuDescriptor();
    current.m_uavs[slot] = handle;
    if (!cachedReflection)
        current.m_rootSigLayout.uavCount = Math::maximum(current.m_rootSigLayout.uavCount, static_cast<U16>(slot+1));
    current.m_resourceTable.uavs = current.m_uavs.data();
    current.setDirty(ContextDirty_Descriptors);
    return (*this);
}


IShaderProgramBinder& D3D12Context::D3D12ShaderProgramBinder::bindSampler(ShaderStageFlags type, U32 slot, GraphicsSampler* sampler)
{
    D3D12Context* context = m_pContext;
    DescriptorHeapAllocationManager* manager = context->getNativeDevice()->getDescriptorHeapManager();
    ContextState& current = currentState();
    D3D12Sampler* pSampler = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = manager->nullSamplerDescriptor();
    if (sampler)
    {
        pSampler = sampler->castTo<D3D12Sampler>();
        handle = pSampler->getDescriptor();
    }
    current.m_samplers[slot] = handle;
    if (!cachedReflection)
        current.m_rootSigLayout.samplerCount = Math::maximum(current.m_rootSigLayout.samplerCount, static_cast<U16>(slot+1));
    current.m_resourceTable.samplers = current.m_samplers.data();
    current.setDirty(ContextDirty_SamplerDescriptors);
    return (*this);
}


void D3D12Context::clearResourceBinds()
{
    ContextState& contextState = m_contextStates.back();
    memset(contextState.m_cbvs.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_cbvs.size());
    memset(contextState.m_srvs.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_srvs.size());
    memset(contextState.m_uavs.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_uavs.size());
    memset(contextState.m_samplers.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_samplers.size());
    if (m_shaderProgramBinder.getReflection())
    {
        // We don't necessarily need to clear out the layout info, since we are currently bound to a program with reflection.
        memset(&contextState.m_rootSigLayout, 0, sizeof(Pipelines::RootSigLayout));
    }
    contextState.m_currentRenderPass = nullptr;
    // contextState.m_currentRootSig = nullptr;
    // contextState.setDirty(ContextDirty_CbvSrvUav);
}


R_INTERNAL
void bindResourceTable(ID3D12GraphicsCommandList* pList, BindType bindType, UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
        switch (bindType)
        {
            case BindType_Graphics:
            {
                pList->SetGraphicsRootDescriptorTable(rootIndex, handle);
                break;
            }
            case BindType_Compute:
            {
                pList->SetComputeRootDescriptorTable(rootIndex, handle);
            }
            default:
            {
                break;
            }
        }
} 


void D3D12Context::bindRootSignature(ID3D12GraphicsCommandList* pList, ContextState& state)
{
    if (state.isDirty(ContextDirty_Descriptors) || state.isDirty(ContextDirty_RootSignature))
    {
        if (state.m_pipelineStateObject.pipelineType == BindType_Graphics)
        {
            state.m_rootSigLayout.flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        }
        ID3D12RootSignature* rootSig = Pipelines::makeRootSignature(m_pDevice, state.m_rootSigLayout);
    
        if (state.m_pipelineStateObject.rootSignature != rootSig)
        {
            state.m_pipelineStateObject.rootSignature = rootSig;
            switch (state.m_pipelineStateObject.pipelineType)
            {
                case BindType_Graphics:
                {
                    pList->SetGraphicsRootSignature(rootSig);
                    break;
                }
                case BindType_Compute:
                {
                    pList->SetComputeRootSignature(rootSig);
                    break;
                }
                default:
                {
                    break;
                }
            }
            // Since our pipeline relies on root signature, any changes will need to retrigger pipeline setup.
            state.setDirty(ContextDirty_Pipeline);
        }
    }
}


void D3D12Context::bindCurrentResources()
{
    ContextState& state = currentState();
    ID3D12GraphicsCommandList* currentList = m_pPrimaryCommandList->get();
    if (state.isDirty(ContextDirty_Descriptors))
    {
        CpuDescriptorTable table = Pipelines::makeDescriptorSrvCbvUavTable(m_pDevice, state.m_rootSigLayout, state.m_resourceTable);
        ShaderVisibleDescriptorHeapInstance* instance = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(currentBufferIndex());
        ShaderVisibleDescriptorTable shaderVisibleTable = instance->upload(m_pDevice->get(), GpuHeapType_CbvSrvUav, table);
        bindResourceTable(currentList, state.m_pipelineStateObject.pipelineType, 0, shaderVisibleTable.baseGpuDescriptorHandle);
    }
    if (state.isDirty(ContextDirty_SamplerDescriptors))
    {
        CpuDescriptorTable table = Pipelines::makeDescriptorSamplertable(m_pDevice, state.m_rootSigLayout, state.m_resourceTable);
        ShaderVisibleDescriptorHeapInstance* instance = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(currentBufferIndex());
        ShaderVisibleDescriptorTable shaderVisibleTable = instance->upload(m_pDevice->get(), GpuHeapType_Sampler, table);
        bindResourceTable(currentList, state.m_pipelineStateObject.pipelineType, 1, shaderVisibleTable.baseGpuDescriptorHandle);
    }
}


IShaderProgramBinder& D3D12Context::bindShaderProgram(ShaderProgramId program, U32 permutation)
{
    D3D::Cache::D3DShaderProgram* shaderProgram = D3D::Cache::obtainShaderProgram(program, permutation);
    R_ASSERT_FORMAT(shaderProgram, "No shader program found by the given id=%d, and permutation=%d", program, permutation);
    if (shaderProgram)
    {
        currentState().m_pipelineStateObject.shaderProgramId = program;
        currentState().m_pipelineStateObject.permutation = permutation;
        currentState().m_pipelineStateObject.pipelineType = shaderProgram->bindType;
        // Descriptors might need to be updated, when we set a new shader program.
        currentState().setDirty(ContextDirty_Pipeline | ContextDirty_RootSignature);
        m_shaderProgramBinder = D3D12ShaderProgramBinder(this, program, permutation);
    }
    return m_shaderProgramBinder;
}


void D3D12Context::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) 
{
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE currentStripValue = currentState().m_pipelineStateObject.graphics.indexStripCut;
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE boundStripValue = getNativeStripCutValue(type);
    if (currentStripValue != boundStripValue)
    {
        currentState().setDirty(ContextDirty_Pipeline);
    }
    // We might want to set this in the internalBindVertexBuffersAndIndexBuffer() call instead, since the binding vertex buffers calls are already made there instead.
    ID3D12Resource* pResource = pIndexBuffer->castTo<D3D12Resource>()->get();
    D3D12_INDEX_BUFFER_VIEW view = { };
    D3D12_RESOURCE_DESC desc = pResource->GetDesc();
    view.BufferLocation = pResource->GetGPUVirtualAddress() + offsetBytes;
    view.SizeInBytes = desc.Width;
    view.Format = type == IndexType_Unsigned16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    currentState().m_indexBufferView = view;
    currentState().setDirty(ContextDirty_IndexBuffer);
}


void D3D12Context::setDepthClampEnable(Bool enable)
{
    currentState().m_pipelineStateObject.graphics.depthClampEnable = enable;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::setDepthBiasEnable(Bool enable)
{
    currentState().m_pipelineStateObject.graphics.depthBiasEnable = enable;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::setDepthCompareOp(CompareOp compare)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.depthCompareOp = compare;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::enableDepthWrite(Bool enable)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.depthWriteEnable = enable;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::setStencilReadMask(U8 mask)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.stencilReadMask = mask;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::setTopology(PrimitiveTopology topology)
{
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = getPrimitiveTopologyType(topology);
    if (currentState().m_pipelineStateObject.graphics.topologyType != topologyType)
    {
        currentState().m_pipelineStateObject.graphics.topologyType = topologyType;
        currentState().setDirty(ContextDirty_Pipeline);
    }
    currentState().m_primitiveTopology = topology;
    currentState().setDirty(ContextDirty_Topology);
}


void D3D12Context::setStencilWriteMask(U8 mask)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.stencilWriteMask = mask;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::setStencilReference(U8 ref)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.stencilReference = ref;
    currentState().setDirty(ContextDirty_StencilRef);
}


void D3D12Context::setColorWriteMask(U32 rtIndex, ColorComponentMaskFlags writeMask)
{
    currentState().m_pipelineStateObject.graphics.blendState.attachments[rtIndex].colorWriteMask = writeMask;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::setInputVertexLayout(VertexInputLayoutId vertexLayout)
{
    VertexInputLayoutId currentId = currentState().m_pipelineStateObject.graphics.inputLayoutId;
    if (currentId != vertexLayout)
    {
        currentState().m_pipelineStateObject.graphics.inputLayoutId = vertexLayout;
        currentState().setDirty(ContextDirty_Pipeline);
    }
}


void D3D12Context::setFrontFace(FrontFace frontFace)
{
    currentState().m_pipelineStateObject.graphics.frontFace = frontFace;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::enableDepth(Bool enable)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.depthTestEnable = enable;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::enableStencil(Bool enable)
{
    currentState().m_pipelineStateObject.graphics.depthStencil.stencilTestEnable = enable;
    currentState().setDirty(ContextDirty_Pipeline);
}


void D3D12Context::bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* offsets)
{
    ID3D12GraphicsCommandList* pList = m_pPrimaryCommandList->get();
    currentState().m_numBoundVertexBuffers = numBuffers;
    for (U32 i = 0; i < numBuffers; ++i)
    {
        D3D12_VERTEX_BUFFER_VIEW bufferView = { };
        D3D12Resource* pVertexBuffer = ppVertexBuffers[i]->castTo<D3D12Resource>();
        ID3D12Resource* resource = pVertexBuffer->get();
        D3D12_RESOURCE_DESC desc = resource->GetDesc();
        bufferView.BufferLocation = resource->GetGPUVirtualAddress() + offsets[i];
        bufferView.SizeInBytes = desc.Width - offsets[i];

        // We will need help from the input layout, to determine this transparently.
        bufferView.StrideInBytes = 0;

        currentState().m_vertexBuffers[i] = bufferView;
    }
    currentState().setDirty(ContextDirty_VertexBuffers);
}


void D3D12Context::internalBindVertexBuffersAndIndexBuffer(ID3D12GraphicsCommandList* list, ContextState& state)
{
    if (state.isDirty(ContextDirty_VertexBuffers))
    {
        for (U32 i = 0; i < state.m_numBoundVertexBuffers; ++i)
        {
            D3D12_VERTEX_BUFFER_VIEW& bufferView = state.m_vertexBuffers[i];
            DeviceId deviceId = getDevice()->castTo<D3D12Device>()->getDeviceId();
            Pipelines::VertexInputs::D3DVertexInput* vertexLayout = Pipelines::VertexInputs::obtain(deviceId, currentState().m_pipelineStateObject.graphics.inputLayoutId);
            R_ASSERT_FORMAT(vertexLayout, "Vertex Layout is required in order to know the stride of the vertex buffer!!");
            bufferView.StrideInBytes = vertexLayout->vertexByteStrides[i];
        }
        list->IASetVertexBuffers(0, state.m_numBoundVertexBuffers, state.m_vertexBuffers.data());
    }
    if (state.isDirty(ContextDirty_IndexBuffer))
    {
        // TODO: Was already set when we called bindIndexBuffer(). We might want to IASetIndexBuffer() here instead.
        list->IASetIndexBuffer(&state.m_indexBufferView);
    }
}


void D3D12Context::bindPipeline(ID3D12GraphicsCommandList* list, ContextState& state)
{
    if (state.isDirty(ContextDirty_Pipeline))
    {
        ID3D12PipelineState* pipelineState = Pipelines::makePipelineState(this, state.m_pipelineStateObject);
        list->SetPipelineState(pipelineState);
    }
    if (state.isDirty(ContextDirty_StencilRef))
    {
        list->OMSetStencilRef(static_cast<UINT>(state.m_pipelineStateObject.graphics.depthStencil.stencilReference));
    }
    if (state.isDirty(ContextDirty_Topology))
    {
        list->IASetPrimitiveTopology(getPrimitiveTopology(state.m_primitiveTopology));
    }
}


ID3D12CommandSignature* D3D12PrimaryCommandList::obtainSignature(D3D12_INDIRECT_ARGUMENT_TYPE type)
{
    auto& iter = m_signatureMap.find(type);
    if (iter == m_signatureMap.end())
    {
        return nullptr;
    }
    return iter->second;
}


void D3D12Context::D3D12ShaderProgramBinder::obtainShaderProgramFromCache()
{
    cachedProgram = D3D::Cache::obtainShaderProgram(getProgramId(), getPermutationId());
    if (cachedProgram)
    {
        cachedReflection = D3D::Cache::obtainShaderProgramReflection(getProgramId(), getPermutationId());
        if (cachedReflection)
        {
            currentState().m_rootSigLayout.cbvCount = (U16)cachedReflection->numCbvs;
            currentState().m_rootSigLayout.srvCount = (U16)cachedReflection->numSrvs;
            currentState().m_rootSigLayout.uavCount = (U16)cachedReflection->numUavs;
            currentState().m_rootSigLayout.samplerCount = (U16)cachedReflection->numSamplers;
        }
    }
}
} // D3D12
} // Recluse