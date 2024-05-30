//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Threading/Threading.hpp"
#include <vector>
#include <map>

namespace Recluse {
namespace D3D12 {

// Allocator for D3D12 resources. This does not allocate the d3d12 memory heap itself,
// only manages it, so be sure to handle the heap creation and destruction outside this
// class!
class D3D12ResourcePagedAllocator 
{
public:
    // Output information used when allocating from this resource allocator.
    struct OutputBlock
    {
        UPtr address; // Starting address of an allocation block.
        class D3D12ResourcePagedAllocator* alloc; // The associated allocator that was used to make this allocation.
    };

    D3D12ResourcePagedAllocator();

    // Initializes this paged allocator. This will request a heap allocation block,
    // to which will use the given Allocator to suballocate with its given method.
    // \param pDevice The D3D12 device to reference for this paged allocator.
    // \param pAllocator The allocator to use for the heap memory that is initialized.
    // \param usage The memory usage that is associated with this memory heap.
    // \param allocateIndex the index id that is associated with this allocator.
    ResultCode initialize(ID3D12Device* pDevice, 
                          Allocator* pAllocator, 
                          U64 totalSizeBytes, 
                          ResourceMemoryUsage usage, 
                          U32 allocatorIndex);
    // Properly cleans up the allocator. This should properly destroy all resources associated with it as well.
    ResultCode release();

    // Performs a suballocation from this allocator's heap, with the given requested info.
    // \param pDevice The devie associated with this allocator.
    // \param allocInfo The request information for allocation.
    // \param blockAddress The starting address of the allocation.
    // \return The result code of the allocation. Any failure will not write to blockAddress.
    ResultCode allocate
                (
                    ID3D12Device* pDevice, 
                    const D3D12_RESOURCE_ALLOCATION_INFO& allocInfo,
                    OutputBlock& blockAddress
                );

    // Frees the memory block that is associated with this allocator. This will block the thread caller, so be sure to use this 
    // sparingly.
    ResultCode free(D3D12MemoryObject* pObject);

    ID3D12Heap* get() const { return m_pool.pHeap; }
    U32         getAllocatorIndex() const { return m_allocatorIndex; }

    // Clear the memory heap allocator, which wipes out any allocated resources. This does not destroy/free the allocator, call release() instead.    
    void    clear();
    
private:
    // The memory pool heap that was allocated on initialization.
    D3D12MemoryPool                 m_pool;
    // The allocator index id.
    U32                             m_allocatorIndex;
    // The allocator that is used to perform the suballocations.
    SmartPtr<Allocator>             m_pAllocator;
    // All garbage resources to be released.
    std::vector<D3D12MemoryObject*> m_garbageResources;
};


// Allocation manager holds onto buffered memory allocators for each context frame. This ensures 
// We don't conflict while one context frame is rendering and the other is being written by the host.
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

    // Call this first when launching a new D3D12 instance, this will initialize the manager.
    ResultCode initialize(ID3D12Device* pDevice);
    // Call this before destroying the D3D12 instance, will free resources and properly destroy the allocation manager.
    ResultCode release();
    
    // Suballocate a memory region from the current context frame.
    ResultCode allocate(D3D12MemoryObject* pOut, 
                        const D3D12_RESOURCE_DESC& desc, 
                        ResourceMemoryUsage usage, 
                        D3D12_CLEAR_VALUE* clearValue, 
                        D3D12_RESOURCE_STATES initialState);
    // Free any suballocated memory called by allocate(). This won't immediately free the
    // memory object, unless you call immediate. Instead, it will queue that memory object
    // until the next time that context frame of which it was called to free(), is back and ready for rendering.
    // This prevents the possibility of freeing that memory while it is in flight. If that is not a worry, then pass immediate=true.
    // \param pObject The memory object that has been allocated by allocate().
    // \param immediate If true, will immediately free the given allocation, otherwise the memory object will be queued and freed the next time
    //                  the same context frame is back.
    ResultCode free(D3D12MemoryObject* pObject, Bool immediate = false);
    // Updates the allocation manager, which allows setting the new frame index, resizing the garbage data structure, and/or managing the 
    // cleanup of resources that were called with free().
    ResultCode update(const Update& update);
    // Reserves memory (host/device) to be used for allocation. 
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
} // D3D12
} // Recluse