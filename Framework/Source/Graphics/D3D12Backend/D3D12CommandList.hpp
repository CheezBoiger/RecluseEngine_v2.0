//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/CommandList.hpp"

namespace Recluse {

class D3D12Device;
class D3D12Context;

class D3D12PrimaryCommandList : public GraphicsCommandList 
{
public:
    
    ResultCode initialize(D3D12Context* pDevice, GraphicsQueueTypeFlags flags);
    ResultCode destroy();

    // Use the given commandlist index.
    void use(U32 bufferIdx);
    void begin() override;
    void end() override;
    void reset() override;

    void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* offsets) override;
    void bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) override;

    void bindDescriptorHeaps(ID3D12DescriptorHeap* const* pHeaps, U32 numHeaps);

    ID3D12GraphicsCommandList* get() { return m_currentCmdList; }

    ID3D12CommandSignature* obtainSignature(D3D12_INDIRECT_ARGUMENT_TYPE type);

private:
    std::vector<ID3D12GraphicsCommandList4*>    m_graphicsCommandLists;
    std::vector<ID3D12CommandAllocator*>        m_allocators;
    ID3D12GraphicsCommandList4*                 m_currentCmdList;
    ID3D12CommandAllocator*                     m_currentAllocator;
    CommandListStatus                           m_status;
    
    std::map<D3D12_INDIRECT_ARGUMENT_TYPE, ID3D12CommandSignature*> m_signatureMap;
};
} // Recluse