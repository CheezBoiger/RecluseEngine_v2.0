//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include "Recluse/Graphics/GraphicsCommon.hpp"

#include <vector>

namespace Recluse {


// Allocator for D3D12 resources. This does not allocate the d3d12 memory heap itself,
// only manages it, so be sure to handle the heap creation and destruction outside this
// class!
class D3D12ResourcePagedAllocator 
{
public:
    D3D12ResourcePagedAllocator();

    ErrType initialize(ID3D12Device* pDevice, Allocator* pAllocator, U64 totalSizeBytes, ResourceMemoryUsage usage);
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
    SmartPtr<Allocator>             m_pAllocator;
};


class D3D12ResourceAllocationManager
{
public:
    static const U64 kAllocationPageSizeBytes;

    D3D12ResourceAllocationManager()
        : m_pDevice(nullptr) 
    { }

    ErrType initialize(ID3D12Device* pDevice);
    
    ErrType allocate(D3D12MemoryObject* pOut, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState);
    ErrType free(D3D12MemoryObject* pObject);
    ErrType update();
    ErrType reserveMemory(const MemoryReserveDescription& description);

private:
    ErrType cleanGarbage(U32 index);

    ID3D12Device* m_pDevice;
    std::vector<D3D12ResourcePagedAllocator*>    m_pagedAllocators;
    // garbage resources to clean up after use.
    std::vector<ID3D12Resource*>            m_garbageToClean;
    U32                                     m_garbageIndex;

    MemoryReserveDescription                       m_description;
};
} // Recluse