//
#include "D3D12Allocator.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


D3D12Allocator::D3D12Allocator(Allocator* pAllocator,
                               D3D12MemoryPool* pPool)
    : m_pAllocator(pAllocator)
    , m_pPool(pPool)
    , m_garbageIndex(0u)
{
    
}



ErrType D3D12Allocator::initialize(U32 garbageBufferCount)
{  
    R_ASSERT(m_pPool            != NULL);
    R_ASSERT(m_pAllocator       != NULL);
    R_ASSERT(garbageBufferCount != 0u);

    m_pAllocator->initialize(0ull, m_pPool->sizeInBytes);

    m_garbageToClean.resize(garbageBufferCount);

    m_garbageIndex = 0u;

    return REC_RESULT_OK;
}


ErrType D3D12Allocator::destroy()
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType D3D12Allocator::allocate
                            (
                                ID3D12Device* pDevice, 
                                D3D12MemoryObject* pOut, 
                                const D3D12_RESOURCE_DESC& desc, 
                                D3D12_RESOURCE_STATES initialState
                            ) 
{
    U64 sizeInBytesRequired = 0ull;
    ErrType result          = REC_RESULT_OK;
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

    if (result != REC_RESULT_OK) 
    {
        R_ERR("D3D12Allocator", "Failed to allocate d3d12 resource!");
    } 
    else 
    {
        HRESULT hresult     = S_OK;
        U64 addressOffset   = allocOut.baseAddress;

        hresult = pDevice->CreatePlacedResource(m_pPool->pHeap, addressOffset, &desc, 
            initState, &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&pOut->pResource);
        
        if (FAILED(hresult)) 
        {
            R_ERR("D3D12Allocator", "Failed to call CreatePlacedResource() on code=(%d)", hresult)
        } 
        else 
        {
            pOut->basePtr       = allocOut.baseAddress;
            pOut->sizeInBytes   = allocOut.sizeBytes;
        }
    }

    return result;
}


ErrType D3D12Allocator::free(D3D12MemoryObject* pObject)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse