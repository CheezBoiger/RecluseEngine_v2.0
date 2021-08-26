//
#include "D3D12CommandList.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12GraphicsCommandList::initialize(D3D12Device* pDevice)
{
    HRESULT result                                      = S_OK;
    const std::vector<BufferResources>& bufferResources = pDevice->getBufferResources();
    ID3D12Device* device                                = pDevice->get();

    m_allocators.resize(bufferResources.size());
    m_graphicsCommandLists.resize(m_allocators.size());

    for (U32 i = 0; i < m_allocators.size(); ++i) {

        m_allocators[i] = bufferResources[i].pAllocator;        
        result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocators[i], 
            nullptr, __uuidof(ID3D12GraphicsCommandList4), (void**)&m_graphicsCommandLists[i]);

        if (FAILED(result)) {
            
            R_ERR(R_CHANNEL_D3D12, "Failed to create d3d12 graphics command list!");
            
            return destroy();

        }
    }

    m_pDevice = pDevice;

    return REC_RESULT_OK;
}


ErrType D3D12GraphicsCommandList::destroy()
{
    return REC_RESULT_OK;
}


void D3D12GraphicsCommandList::begin()
{
    R_ASSERT(m_pDevice != NULL);
    R_ASSERT(!m_graphicsCommandLists.empty());
    R_ASSERT(!m_allocators.empty());

    U32 currentBufferInd    = m_pDevice->getCurrentBufferIndex();
    m_currentCmdList        = m_graphicsCommandLists[currentBufferInd];
    m_currentAllocator      = m_allocators[currentBufferInd];

    m_currentCmdList->Reset(m_currentAllocator, nullptr);
}


void D3D12GraphicsCommandList::end()
{
    m_currentCmdList->Close();
}
} // Recluse