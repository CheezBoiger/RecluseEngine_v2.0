//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Threading/Threading.hpp"
#include <vector>
#include <map>

namespace Recluse {


// Allocator for D3D12 resources. This does not allocate the d3d12 memory heap itself,
// only manages it, so be sure to handle the heap creation and destruction outside this
// class!
class D3D12ResourcePagedAllocator 
{
public:
    D3D12ResourcePagedAllocator();

    ResultCode initialize(ID3D12Device* pDevice, Allocator* pAllocator, U64 totalSizeBytes, ResourceMemoryUsage usage);
    ResultCode release();

    ResultCode allocate
                (
                    ID3D12Device* pDevice, 
                    D3D12MemoryObject* pOut, 
                    const D3D12_RESOURCE_DESC& desc, 
                    D3D12_RESOURCE_STATES initialState
                );

    ResultCode free(D3D12MemoryObject* pObject);
    
    void    clear();
    
private:
    
    D3D12MemoryPool                 m_pool;
    SmartPtr<Allocator>             m_pAllocator;
    CriticalSection                 m_allocateCs;
    std::vector<D3D12MemoryObject*> m_garbageResources;
};


class D3D12ResourceAllocationManager
{
public:
    static const U64 kAllocationPageSizeBytes;

    D3D12ResourceAllocationManager()
        : m_pDevice(nullptr) 
    { }

    ResultCode initialize(ID3D12Device* pDevice);
    
    ResultCode allocate(D3D12MemoryObject* pOut, const D3D12_RESOURCE_DESC& desc, ResourceMemoryUsage usage, D3D12_RESOURCE_STATES initialState);
    ResultCode free(D3D12MemoryObject* pObject);
    ResultCode update();
    ResultCode reserveMemory(const MemoryReserveDescription& description);

private:
    ResultCode cleanGarbage(U32 index);

    ID3D12Device*                                                               m_pDevice;
    std::map<ResourceMemoryUsage, std::vector<D3D12ResourcePagedAllocator*>>    m_pagedAllocators;
    U32                                                                         m_garbageIndex;
    MemoryReserveDescription                                                    m_description;
};
} // Recluse