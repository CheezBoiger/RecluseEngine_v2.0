//
#include "D3D12Allocator.hpp"
#include "D3D12Commons.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {

const U64 D3D12ResourceAllocationManager::kAllocationPageSizeBytes = R_MB(64);


D3D12ResourcePagedAllocator::D3D12ResourcePagedAllocator()
    : m_pAllocator(nullptr)
{
    
}


ErrType D3D12ResourcePagedAllocator::initialize(ID3D12Device* pDevice, Allocator* pAllocator, U64 totalSizeBytes, ResourceMemoryUsage usage)
{  
    R_ASSERT(pAllocator         != NULL);
    R_ASSERT(totalSizeBytes     != 0u);

    D3D12_HEAP_TYPE heapType                = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_CPU_PAGE_PROPERTY cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

    switch (usage)
    {
    case ResourceMemoryUsage_CpuOnly:
        heapType        = D3D12_HEAP_TYPE_UPLOAD;
        cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
        break;
    case ResourceMemoryUsage_CpuToGpu:
        heapType        = D3D12_HEAP_TYPE_UPLOAD;
        cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
        break;
    case ResourceMemoryUsage_GpuToCpu:
        heapType        = D3D12_HEAP_TYPE_READBACK;
        cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
        break;
    case ResourceMemoryUsage_GpuOnly:
    default:
        heapType        = D3D12_HEAP_TYPE_DEFAULT;
        cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
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

    HRESULT result = pDevice->CreateHeap(&heapDesc, __uuidof(ID3D12Heap), (void**)&m_pool.pHeap);

    if (FAILED(result))
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to create directX heap! ErrCode=%d", result);
        return;
    }

    m_pool.sizeInBytes = totalSizeBytes;
    m_pAllocator = makeSmartPtr(pAllocator);
    m_pAllocator->initialize(0ull, totalSizeBytes);
    return RecluseResult_Ok;
}


ErrType D3D12ResourcePagedAllocator::release()
{
    return RecluseResult_NoImpl;
}


ErrType D3D12ResourcePagedAllocator::allocate
                            (
                                ID3D12Device* pDevice, 
                                D3D12MemoryObject* pOut, 
                                const D3D12_RESOURCE_DESC& desc, 
                                D3D12_RESOURCE_STATES initialState
                            ) 
{
    U64 sizeInBytesRequired = 0ull;
    ErrType result          = RecluseResult_Ok;
    SIZE_T formatSizeBytes  = Dxgi::getNativeFormatSize(desc.Format);
    U64 alignment           = desc.Alignment;
    U32 width               = desc.Width;
    U32 height              = desc.Height;
    U32 depthOrArray        = desc.DepthOrArraySize;
    U32 samples             = desc.SampleDesc.Count;
    U32 mips                = desc.MipLevels;

    const D3D12_RESOURCE_STATES initState   = initialState;
    D3D12_CLEAR_VALUE optimizedClearValue   = { };

    R_ASSERT(sizeInBytesRequired != 0ull);

    Allocation allocOut     = { };
    result = m_pAllocator->allocate(&allocOut, sizeInBytesRequired, alignment);

    if (result != RecluseResult_Ok) 
    {
        R_ERR("D3D12Allocator", "Failed to allocate d3d12 resource!");
    } 
    else 
    {
        HRESULT hresult     = S_OK;
        U64 addressOffset   = allocOut.baseAddress;

        hresult = pDevice->CreatePlacedResource(m_pool.pHeap, addressOffset, &desc, 
            initState, &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&pOut->pResource);
        
        if (FAILED(hresult)) 
        {
            R_ERR("D3D12Allocator", "Failed to call CreatePlacedResource() on code=(%d)", hresult);
        } 
        else 
        {
            pOut->basePtr       = allocOut.baseAddress;
            pOut->sizeInBytes   = allocOut.sizeBytes;
        }
    }

    return result;
}


ErrType D3D12ResourcePagedAllocator::free(D3D12MemoryObject* pObject)
{
    R_ASSERT(pObject != NULL);

    Allocation allocation   = { };
    allocation.baseAddress  = pObject->basePtr;
    allocation.sizeBytes    = pObject->sizeInBytes;
    ErrType err = m_pAllocator->free(&allocation);
    
    if (err != RecluseResult_Ok)
    {
        return err;
    }

    pObject->pResource->Release();

    return RecluseResult_NoImpl;
}


void D3D12ResourcePagedAllocator::clear()
{
    R_ASSERT(m_pool.pHeap != NULL);
    R_ASSERT(m_pAllocator != NULL);
    R_ASSERT(m_pool.sizeInBytes != 0ull);

    m_pAllocator->reset();
}

ErrType D3D12ResourceAllocationManager::initialize(ID3D12Device* pDevice)
{
    m_pDevice = pDevice;
    return RecluseResult_Ok;
}


ErrType D3D12ResourceAllocationManager::allocate(D3D12MemoryObject* pOut, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState)
{

    return RecluseResult_NoImpl;
}


ErrType D3D12ResourceAllocationManager::free(D3D12MemoryObject* pObject)
{
    return RecluseResult_NoImpl;
}


ErrType D3D12ResourceAllocationManager::update()
{
    cleanGarbage(m_garbageIndex);
    return RecluseResult_Ok;
}


ErrType D3D12ResourceAllocationManager::cleanGarbage(U32 index)
{

}
} // Recluse