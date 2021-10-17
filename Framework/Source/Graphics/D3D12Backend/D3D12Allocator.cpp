//
#include "D3D12Allocator.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


D3D12Allocator::D3D12Allocator(Allocator* pAllocator)
    : m_pAllocator(pAllocator)
    , m_pHeap(nullptr)
{
}



ErrType D3D12Allocator::initialize()
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType D3D12Allocator::destroy()
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType D3D12Allocator::allocate(ID3D12Resource** pResource, U64 szBytes, U64 alignment)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType D3D12Allocator::free(ID3D12Resource* pResource)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse