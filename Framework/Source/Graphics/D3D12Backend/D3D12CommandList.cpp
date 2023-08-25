//
#include "D3D12CommandList.hpp"
#include "D3D12Device.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

namespace Recluse {


ResultCode D3D12PrimaryCommandList::initialize(D3D12Context* pDeviceContext, GraphicsQueueTypeFlags flags)
{
    flags;

    HRESULT result                                      = S_OK;
    const std::vector<BufferResources>& bufferResources = pDeviceContext->getBufferResources();
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

    return RecluseResult_Ok;
}


ResultCode D3D12PrimaryCommandList::destroy()
{
    for (U32 i = 0; i < m_graphicsCommandLists.size(); ++i)
    {
        m_graphicsCommandLists[i]->Release();
    }
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
    ID3D12GraphicsCommandList* pList = get();
}


void D3D12PrimaryCommandList::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type)
{

}


void D3D12PrimaryCommandList::bindDescriptorHeaps(ID3D12DescriptorHeap* const* pHeaps, U32 numHeaps)
{
    get()->SetDescriptorHeaps(numHeaps, pHeaps);
}


void D3D12Context::drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance)
{
    bindCurrentResources();
    currentState().setClean();
}


void D3D12Context::bindShaderResource(ShaderType type, U32 slot, ResourceViewId view)
{
    ContextState& current = currentState();
    D3D12GraphicsResourceView* pView = DescriptorViews::findResourceView(view);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = pView->getCpuDescriptor();
    current.m_srvs[slot] = handle;
    current.m_rootSigLayout.srvCount = Math::maximum(current.m_rootSigLayout.srvCount, static_cast<U16>(slot+1));
    current.m_resourceTable.srvs = current.m_srvs.data();
    current.setDirty(ContextDirty_Descriptors);
}


void D3D12Context::bindConstantBuffer(ShaderType type, U32 slot, GraphicsResource* pResource, U32 offsetBytes, U32 sizeBytes)
{
    ContextState& current = currentState();
    D3D12Resource* pNativeResource = pResource->castTo<D3D12Resource>();
    R_ASSERT(pNativeResource);
    D3D12_CPU_DESCRIPTOR_HANDLE cbv = pNativeResource->asCbv(offsetBytes, sizeBytes);
    current.m_cbvs[slot] = cbv;
    current.m_rootSigLayout.cbvCount = Math::maximum(current.m_rootSigLayout.cbvCount, static_cast<U16>(slot+1));
    current.m_resourceTable.cbvs = current.m_cbvs.data();
    current.setDirty(ContextDirty_Descriptors);
}


void D3D12Context::bindUnorderedAccessView(ShaderType type, U32 slot, ResourceViewId view)
{
    ContextState& current = currentState();
    D3D12GraphicsResourceView* pView = DescriptorViews::findResourceView(view);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = pView->getCpuDescriptor();
    current.m_uavs[slot] = handle;
    current.m_rootSigLayout.uavCount = Math::maximum(current.m_rootSigLayout.uavCount, static_cast<U16>(slot+1));
    current.m_resourceTable.uavs = current.m_uavs.data();
    current.setDirty(ContextDirty_Descriptors);
}


void D3D12Context::clearResourceBinds()
{
    ContextState& contextState = m_contextStates.back();
    memset(contextState.m_cbvs.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_cbvs.size());
    memset(contextState.m_srvs.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_srvs.size());
    memset(contextState.m_uavs.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_uavs.size());
    memset(contextState.m_samplers.data(), 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * contextState.m_samplers.size());
    memset(&contextState.m_rootSigLayout, 0, sizeof(Pipelines::RootSigLayout));
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
                pList->SetGraphicsRootDescriptorTable(0, handle);
                break;
            }
            case BindType_Compute:
            {
                pList->SetComputeRootDescriptorTable(0, handle);
            }
            default:
            {
                break;
            }
        }
} 


R_INTERNAL
void bindRootSignature(ID3D12GraphicsCommandList* pList, BindType bindType, ID3D12RootSignature* rootSig)
{
    switch (bindType)
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
}


void D3D12Context::bindCurrentResources()
{
    ContextState& state = currentState();
    if (state.isDirty(ContextDirty_Descriptors))
    {
        ID3D12GraphicsCommandList* currentList = m_pPrimaryCommandList->get();
        ID3D12RootSignature* rootSig = Pipelines::makeRootSignature(m_pDevice, state.m_rootSigLayout);
        CpuDescriptorTable table = Pipelines::makeDescriptorSrvCbvUavTable(m_pDevice, state.m_rootSigLayout, state.m_resourceTable);
        ShaderVisibleDescriptorHeapInstance* instance = m_pDevice->getDescriptorHeapManager()->getShaderVisibleInstance(currentBufferIndex());
        ShaderVisibleDescriptorTable shaderVisibleTable = instance->upload(m_pDevice->get(), GpuHeapType_CbvSrvUav, table);

        if (state.m_currentRootSig != rootSig)
        {
            bindRootSignature(currentList, state.m_bindType, rootSig);
            state.m_currentRootSig = rootSig;
        }

        bindResourceTable(currentList, state.m_bindType, 0, shaderVisibleTable.baseGpuDescriptorHandle);
    }
}
} // Recluse