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
    struct OutputBlock
    {
        UPtr address;
        class D3D12ResourcePagedAllocator* alloc;
    };

    D3D12ResourcePagedAllocator();

    ResultCode initialize(ID3D12Device* pDevice, Allocator* pAllocator, U64 totalSizeBytes, ResourceMemoryUsage usage, U32 allocatorIndex);
    ResultCode release();

    ResultCode allocate
                (
                    ID3D12Device* pDevice, 
                    const D3D12_RESOURCE_ALLOCATION_INFO& allocInfo,
                    OutputBlock& blockAddress
                );

    ResultCode free(D3D12MemoryObject* pObject);
    ID3D12Heap* get() const { return m_pool.pHeap; }
    U32         getAllocatorIndex() const { return m_allocatorIndex; }
    
    void    clear();
    
private:
    
    D3D12MemoryPool                 m_pool;
    U32                             m_allocatorIndex;
    SmartPtr<Allocator>             m_pAllocator;
    std::vector<D3D12MemoryObject*> m_garbageResources;
};


class D3D12ResourceAllocationManager
{
public:
    static const U64 kAllocationPageSizeBytes;

    enum UpdateFlag
    {
        UpdateFlag_Update = (1 << 0),
        UpdateFlag_ResizeGarbage = (1 << 1),
        UpdateFlag_SetFrameIndex = (1 << 2)
    };
    typedef U32 UpdateFlags;

    struct Update
    {
        UpdateFlags     flags;
        U16             frameIndex;
        U16             frameSize;
    };

    D3D12ResourceAllocationManager()
        : m_pDevice(nullptr) 
    { }

    ResultCode initialize(ID3D12Device* pDevice);
    ResultCode release();
    
    ResultCode allocate(D3D12MemoryObject* pOut, const D3D12_RESOURCE_DESC& desc, ResourceMemoryUsage usage, D3D12_CLEAR_VALUE* clearValue, D3D12_RESOURCE_STATES initialState);
    ResultCode free(D3D12MemoryObject* pObject, Bool immediate = false);
    ResultCode update(const Update& update);
    ResultCode reserveMemory(const MemoryReserveDescription& description);

private:
    ResultCode cleanGarbage(U32 index);

    ID3D12Device*                                                                       m_pDevice;
    std::map<ResourceMemoryUsage, std::vector<SmartPtr<D3D12ResourcePagedAllocator>>>   m_pagedAllocators;
    U32                                                                                 m_garbageIndex;
    std::vector<std::vector<D3D12MemoryObject>>                                         m_garbage;
    MemoryReserveDescription                                                            m_description;
    CriticalSection                                                                     m_allocateCs;
};
} // Recluse