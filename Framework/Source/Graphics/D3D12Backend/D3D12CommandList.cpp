//
#include "D3D12CommandList.hpp"
#include "D3D12Device.hpp"
#include "D3D12Resource.hpp"
#include "D3D12ResourceView.hpp"

#include "Recluse/Messaging.hpp"

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

    m_pDeviceContext = pDeviceContext;

    return RecluseResult_Ok;
}


ResultCode D3D12PrimaryCommandList::destroy()
{
    return RecluseResult_Ok;
}


void D3D12PrimaryCommandList::begin()
{
    R_ASSERT(m_pDeviceContext != NULL);
    R_ASSERT(!m_graphicsCommandLists.empty());
    R_ASSERT(!m_allocators.empty());

    U32 currentBufferInd    = m_pDeviceContext->getCurrentBufferIndex();
    m_currentCmdList        = m_graphicsCommandLists[currentBufferInd];
    m_currentAllocator      = m_allocators[currentBufferInd];

    m_currentCmdList->Reset(m_currentAllocator, nullptr);
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
    
}


void D3D12PrimaryCommandList::bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type)
{

}
} // Recluse