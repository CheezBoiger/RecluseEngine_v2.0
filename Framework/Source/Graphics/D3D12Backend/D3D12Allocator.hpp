//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include <vector>

namespace Recluse {


// Allocator for D3D12 resources. This does not allocate the d3d12 memory heap itself,
// only manages it, so be sure to handle the heap creation and destruction outside this
// class!
class D3D12ResourceAllocator 
{
public:
    D3D12ResourceAllocator();

    ErrType initialize(ID3D12Device* pDevice, U64 totalSizeBytes);
    ErrType release();

    ErrType allocate
                (
                    ID3D12Device* pDevice, 
                    D3D12MemoryObject* pOut, 
                    const D3D12_RESOURCE_DESC& desc, 
                    D3D12_RESOURCE_STATES initialState
                );

    ErrType free(D3D12MemoryObject* pObject);
    
    void    clear();
    
private:
    D3D12MemoryPool                 m_pool;
    Allocator*                      m_pAllocator;
};


class D3D12ResourceAllocationManager
{
public:
    static const U64 kAllocationPageSizeBytes;

    ErrType initialize(ID3D12Device* pDevice);
    
    ErrType allocate(D3D12MemoryObject* pOut, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState);
    ErrType free(D3D12MemoryObject* pObject);
    ErrType cleanGarbage(U32 index);

private:
    std::vector<D3D12ResourceAllocator*>    m_allocators;
    // garbage resources to clean up after use.
    std::vector<ID3D12Resource*>            m_garbageToClean;
    U32                                     m_garbageIndex;
};
} // Recluse