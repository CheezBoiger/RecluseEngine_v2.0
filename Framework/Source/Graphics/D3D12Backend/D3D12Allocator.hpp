//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include <vector>

namespace Recluse {


// Allocator for D3D12 resources. This does not allocate the d3d12 memory heap itself,
// only manages it, so be sure to handle the heap creation and destruction outside this
// class!
class D3D12Allocator 
{
public:
    D3D12Allocator
        (
            Allocator* pAllocator = nullptr,
            D3D12MemoryPool* pPool = nullptr
        );

    void    setMemoryPool(D3D12MemoryPool* pPool) { m_pPool = pPool; }
    ErrType initialize(U32 garbageBufferCount);
    ErrType destroy();

    ErrType allocate
                (
                    ID3D12Device* pDevice, 
                    D3D12MemoryObject* pOut, 
                    const D3D12_RESOURCE_DESC& desc, 
                    D3D12_RESOURCE_STATES initialState
                );

    ErrType free(D3D12MemoryObject* pObject);

    ErrType cleanGarbage(U32 index);

    void    clear();
    
private:
    D3D12MemoryPool*                m_pPool;
    Allocator*                      m_pAllocator;

    // garbage resources to clean up after use.
    std::vector<ID3D12Resource*>    m_garbageToClean;
    U32                             m_garbageIndex;
};
} // Recluse