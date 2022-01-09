//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/CommandList.hpp"

namespace Recluse {

class D3D12Device;

class D3D12CommandList : public GraphicsCommandList 
{
public:
    
    ErrType initialize(D3D12Device* pDevice, GraphicsQueueTypeFlags flags);
    ErrType destroy();

    void begin() override;
    void end() override;

    ID3D12GraphicsCommandList* get() { return m_currentCmdList; }

private:
    std::vector<ID3D12GraphicsCommandList4*>    m_graphicsCommandLists;
    std::vector<ID3D12CommandAllocator*>        m_allocators;
    ID3D12GraphicsCommandList4*                 m_currentCmdList;
    ID3D12CommandAllocator*                     m_currentAllocator;
    D3D12Device* m_pDevice;
};
} // Recluse