//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Memory/BuddyAllocator.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include <vector>
#include <map>

namespace Recluse {
namespace D3D12 {


// Allocator for D3D12 resources. This does not allocate the d3d12 memory heap itself,
// only manages it, so be sure to handle the heap creation and destruction outside this
// class!
template<typename AllocatorContext>
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
    Allocator*                      m_pAllocator;
    // All garbage resources to be released.
    std::vector<D3D12MemoryObject*> m_garbageResources;

    AllocatorContext                m_alloc;
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

    struct BuddyAllocationContext
    {
        Allocator* create() { return new BuddyAllocator(); }
        void destroy(Allocator* allocator) { delete allocator; }
    };

    ID3D12Device*                                                                       m_pDevice;
    std::map<ResourceMemoryUsage, std::vector<SmartPtr<D3D12ResourcePagedAllocator<BuddyAllocationContext>>>>   m_pagedAllocators;
    U32                                                                                 m_garbageIndex;
    std::vector<std::vector<D3D12MemoryObject>>                                         m_garbage;
    MemoryReserveDescription                                                            m_description;
    CriticalSection                                                                     m_allocateCs;
};


// temporary allocator used for temporary allocations!!
class D3D12TemporaryBufferAllocator
{
public:
    static const U64 kTemporaryAllocationPageSizeBytes;

    ResultCode initialize(ID3D12Device* device);
    ResultCode release();

    ResultCode allocate(D3D12MemoryObject* pOut, ResourceMemoryUsage usage, U32 cbSizeBytes);

    // Clear out the temporary resources. Starting fresh.
    ResultCode clear();

private:
    struct LinearAllocationContext
    {
        Allocator* create() { return new LinearAllocator(); }
        void destroy(Allocator* allocator) { delete allocator; }
    };

    struct BufferAllocatorContext
    {
        ID3D12Resource*                                                 baseResource;
        LinearAllocator                                                 allocator;

        BufferAllocatorContext(ID3D12Device* device, ResourceMemoryUsage usage, U64 sizeBytes, U32 allocatorIndex)
        {
            D3D12_RESOURCE_DESC resourceDesc    = { };

            D3D12_HEAP_TYPE heapType                = D3D12_HEAP_TYPE_DEFAULT;
            D3D12_CPU_PAGE_PROPERTY cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

            switch (usage)
            {
                case ResourceMemoryUsage_CpuVisible:
                    heapType        = D3D12_HEAP_TYPE_UPLOAD;
                    cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    break;
                case ResourceMemoryUsage_CpuToGpu:
                    heapType        = D3D12_HEAP_TYPE_UPLOAD;
                    cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    break;
                case ResourceMemoryUsage_GpuToCpu:
                    heapType        = D3D12_HEAP_TYPE_READBACK;
                    cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    break;
                case ResourceMemoryUsage_GpuOnly:
                default:
                    heapType        = D3D12_HEAP_TYPE_DEFAULT;
                    cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    break;
            }
            D3D12_HEAP_PROPERTIES properties        = { };
            properties.Type                = heapType;
            properties.CPUPageProperty     = cpuPageProperty;
            properties.CreationNodeMask    = 0;
            properties.VisibleNodeMask     = 0;
            properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

            resourceDesc.Width = align(sizeBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.MipLevels = 1;
            resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            resourceDesc.SampleDesc = { 1, 0 };
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            device->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, __uuidof(ID3D12Resource), (void**)&baseResource);
            allocator.initialize(0, sizeBytes);
        }

        ~BufferAllocatorContext()
        {
        }

        void release()
        {
            if (baseResource)
                baseResource->Release();
            baseResource = nullptr;
            allocator.cleanUp();
        }
    };
    ID3D12Device* m_pDevice;
    std::map<ResourceMemoryUsage, std::vector<BufferAllocatorContext>> m_pagedAllocators;
    CriticalSection m_cs;
};
} // D3D12
} // Recluse