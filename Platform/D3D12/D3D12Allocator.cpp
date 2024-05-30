//
#include "D3D12Allocator.hpp"
#include "D3D12Commons.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/BuddyAllocator.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace D3D12 {
const U64 D3D12ResourceAllocationManager::kAllocationPageSizeBytes = R_MB(64);


D3D12ResourcePagedAllocator::D3D12ResourcePagedAllocator()
    : m_pAllocator(nullptr)
{
    
}


ResultCode D3D12ResourcePagedAllocator::initialize(ID3D12Device* pDevice, Allocator* pAllocator, U64 totalSizeBytes, ResourceMemoryUsage usage, U32 allocatorIndex)
{  
    R_ASSERT(pAllocator         != NULL);
    R_ASSERT(totalSizeBytes     != 0u);

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
    
    D3D12_HEAP_DESC heapDesc                = { };
    heapDesc.Alignment                      = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.SizeInBytes                    = totalSizeBytes;
    heapDesc.Flags                          = D3D12_HEAP_FLAG_NONE;
    heapDesc.Properties.Type                = heapType;
    heapDesc.Properties.CPUPageProperty     = cpuPageProperty;
    heapDesc.Properties.CreationNodeMask    = 0;
    heapDesc.Properties.VisibleNodeMask     = 0;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    HRESULT result = pDevice->CreateHeap(&heapDesc, __uuidof(ID3D12Heap), (void**)&m_pool.pHeap);

    if (FAILED(result))
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create Direct3D 12 heap! ErrCode=%d", result);
        return RecluseResult_Failed;
    }

    m_pool.sizeInBytes = totalSizeBytes;
    m_allocatorIndex = allocatorIndex;
    m_pAllocator = makeSmartPtr(pAllocator);
    m_pAllocator->initialize(0ull, totalSizeBytes);
    return RecluseResult_Ok;
}


ResultCode D3D12ResourcePagedAllocator::release()
{
    if (m_pAllocator)
    {
        m_pAllocator->reset();
        m_pAllocator->cleanUp();
        m_pAllocator.release();
    }
    
    if (m_pool.pHeap)
    {
        m_pool.pHeap->Release();
        m_pool.pHeap = nullptr;
    }
    return RecluseResult_NoImpl;
}


ResultCode D3D12ResourcePagedAllocator::allocate
                            (
                                ID3D12Device* pDevice,
                                const D3D12_RESOURCE_ALLOCATION_INFO& allocInfo,
                                OutputBlock& outBlock
                            ) 
{
    ResultCode result                                       = RecluseResult_Ok;

    UPtr address = m_pAllocator->allocate(allocInfo.SizeInBytes, allocInfo.Alignment);
    result = m_pAllocator->getLastError();

    outBlock.address = address;
    outBlock.alloc = this; 

    return result;
}


ResultCode D3D12ResourcePagedAllocator::free(D3D12MemoryObject* pObject)
{
    R_ASSERT(pObject != NULL);

    ResultCode err = RecluseResult_Ok;
    
    m_pAllocator->free(pObject->basePtr);
    
    if (err != RecluseResult_Ok)
    {
        return err;
    }

    pObject->pResource->Release();
    R_DEBUG(R_CHANNEL_D3D12, "Freed memory object.");
    return err;
}


void D3D12ResourcePagedAllocator::clear()
{
    R_ASSERT(m_pool.pHeap != NULL);
    R_ASSERT(m_pAllocator != NULL);
    R_ASSERT(m_pool.sizeInBytes != 0ull);

    m_pAllocator->reset();
}

ResultCode D3D12ResourceAllocationManager::initialize(ID3D12Device* pDevice)
{
    m_pDevice = pDevice;
    m_allocateCs.initialize();
    return RecluseResult_Ok;
}


ResultCode D3D12ResourceAllocationManager::allocate(D3D12MemoryObject* pOut, const D3D12_RESOURCE_DESC& desc, ResourceMemoryUsage usage, D3D12_CLEAR_VALUE* clearValue, D3D12_RESOURCE_STATES initialState)
{
    std::vector<SmartPtr<D3D12ResourcePagedAllocator>>& pagedAllocators = m_pagedAllocators[usage];
    D3D12_RESOURCE_ALLOCATION_INFO resourceAllocationInfo   = m_pDevice->GetResourceAllocationInfo(0, 1, &desc);
    ScopedCriticalSection _(m_allocateCs);
    if (pagedAllocators.empty())
    {
        pagedAllocators.push_back(new D3D12ResourcePagedAllocator());
        pagedAllocators.back()->initialize(m_pDevice, new BuddyAllocator(), Math::maximum(align(resourceAllocationInfo.SizeInBytes, resourceAllocationInfo.Alignment), kAllocationPageSizeBytes), usage, pagedAllocators.size() - 1u); 
    }
    
    D3D12ResourcePagedAllocator* pagedAllocator = pagedAllocators.back();
    D3D12ResourcePagedAllocator::OutputBlock outputBlock = { };
    ResultCode result = pagedAllocator->allocate(m_pDevice, resourceAllocationInfo, outputBlock);


    if (result == RecluseResult_OutOfMemory)
    {
        U64 newSizeChunk = Math::maximum(align(resourceAllocationInfo.SizeInBytes, resourceAllocationInfo.Alignment), kAllocationPageSizeBytes);
        pagedAllocators.push_back(new D3D12ResourcePagedAllocator());
        pagedAllocators.back()->initialize(m_pDevice, new BuddyAllocator(), kAllocationPageSizeBytes, usage, pagedAllocators.size() - 1u);
        result = pagedAllocators.back()->allocate(m_pDevice, resourceAllocationInfo, outputBlock);
    }
    if (result != RecluseResult_Ok)
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to allocate!!");
    }

    if (result == RecluseResult_Ok)
    {
        if (result != RecluseResult_Ok) 
    {
        R_ERROR("D3D12Allocator", "Failed to allocate d3d12 resource!");
    } 
    else 
    {
        HRESULT hresult     = S_OK;
        UPtr addressOffset   = outputBlock.address;
    
        hresult = m_pDevice->CreatePlacedResource(outputBlock.alloc->get(), addressOffset, &desc, 
            initialState, clearValue, __uuidof(ID3D12Resource), (void**)&pOut->pResource);
        
        if (FAILED(hresult)) 
        {
            R_ERROR("D3D12Allocator", "Failed to call CreatePlacedResource() on code=(%d)", hresult);
        } 
        else 
        {
            pOut->basePtr           = addressOffset;
            pOut->sizeInBytes       = resourceAllocationInfo.SizeInBytes;
            pOut->allocatorIndex    = outputBlock.alloc->getAllocatorIndex();
        }
    }
    }

    pOut->usage = usage;
    return result;
}


ResultCode D3D12ResourceAllocationManager::free(D3D12MemoryObject* pObject, Bool immediate)
{
    if (!pObject)
    {
        return RecluseResult_NullPtrExcept;
    }
    ResultCode result = RecluseResult_Ok;
    if (!immediate)
    {
        m_garbage[m_garbageIndex].push_back(*pObject);
    }
    else
    {
        D3D12MemoryObject& object = *pObject;
        std::vector<SmartPtr<D3D12ResourcePagedAllocator>>& pagedAllocators = m_pagedAllocators[object.usage];
        D3D12ResourcePagedAllocator* pagedAllocator = pagedAllocators[object.allocatorIndex];
        result = pagedAllocator->free(&object);
    }
    return result;
}


ResultCode D3D12ResourceAllocationManager::update(const Update& update)
{
    if (update.flags & UpdateFlag_ResizeGarbage)
    {
        for (U32 i = 0; i < m_garbage.size(); ++i)
            cleanGarbage(i);
        m_garbage.resize(update.frameSize);
    }

    if (update.flags & UpdateFlag_SetFrameIndex)
    {
        m_garbageIndex = update.frameIndex;
    }

    if (update.flags & UpdateFlag_Update)
    {
        cleanGarbage(m_garbageIndex);
    }
    return RecluseResult_Ok;
}


ResultCode D3D12ResourceAllocationManager::cleanGarbage(U32 index)
{
    std::vector<D3D12MemoryObject>& garbage = m_garbage[index];
    for (U32 i = 0; i < garbage.size(); ++i)
    {
        D3D12MemoryObject& object = garbage[i];
        std::vector<SmartPtr<D3D12ResourcePagedAllocator>>& pagedAllocators = m_pagedAllocators[object.usage];
        D3D12ResourcePagedAllocator* pagedAllocator = pagedAllocators[object.allocatorIndex];
        pagedAllocator->free(&object);
    }
    garbage.clear();
    return RecluseResult_NoImpl;
}


ResultCode D3D12ResourceAllocationManager::reserveMemory(const MemoryReserveDescription& description)
{
    m_description = description;
    return RecluseResult_Ok;
}


ResultCode D3D12ResourceAllocationManager::release()
{
    m_allocateCs.release();

    for (U32 i = 0; i < m_garbage.size(); ++i)
    {
        cleanGarbage(i);
    }
    m_garbage.clear();

    for (auto pagedAllocators : m_pagedAllocators)
    {
        for (auto heap : pagedAllocators.second)
        {
            heap->release();
        }
    }
    m_pagedAllocators.clear();
    return RecluseResult_Ok;
}
} // D3D12
} // Recluse