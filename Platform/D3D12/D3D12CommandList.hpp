//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/CommandList.hpp"

#define R_D3D12_COMMANDLIST_IMPL 1
namespace Recluse {
namespace D3D12 {
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

    ID3D12GraphicsCommandList* get() { return m_currentCmdList->get6(); }
    ID3D12GraphicsCommandList6* get6() { return m_currentCmdList->get6(); }
    
#if defined(__ID3D12WorkGraphProperties_FWD_DEFINED__)
    ID3D12GraphicsCommandList10* get10() { return m_currentCmdList->get10(); }
#endif

    ID3D12CommandSignature* obtainSignature(D3D12_INDIRECT_ARGUMENT_TYPE type);

private:

    struct CommandListImpl
    {
        ID3D12GraphicsCommandList6* list6;
        
        ID3D12GraphicsCommandList6* get6() const { return list6; }

#if defined(__ID3D12WorkGraphProperties_FWD_DEFINED__)
        ID3D12GraphicsCommandList10* list10;

        ID3D12GraphicsCommandList10* get10() const { return list10; }
#endif
        CommandListImpl() : list6(nullptr) { }

        HRESULT     initialize(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator);
        void        release();
    };
#if defined(R_D3D12_COMMANDLIST_IMPL)
    std::vector<CommandListImpl>                m_graphicsCommandLists;
#else
    std::vector<ID3D12GraphicsCommandList6*>    m_graphicsCommandLists;
#endif
    std::vector<ID3D12CommandAllocator*>        m_allocators;

#if defined(R_D3D12_COMMANDLIST_IMPL)
    CommandListImpl*                            m_currentCmdList;
#else
    ID3D12GraphicsCommandList6*                 m_currentCmdList;
#endif
    ID3D12CommandAllocator*                     m_currentAllocator;
    CommandListStatus                           m_status;
    
    std::map<D3D12_INDIRECT_ARGUMENT_TYPE, ID3D12CommandSignature*> m_signatureMap;
};
} // D3D12
} // Recluse