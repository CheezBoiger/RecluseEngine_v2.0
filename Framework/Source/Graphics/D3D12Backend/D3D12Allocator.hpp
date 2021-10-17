//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Memory/Allocator.hpp"

#include <vector>

namespace Recluse {


class D3D12Allocator {
public:
    D3D12Allocator(Allocator* pAllocator = nullptr);

    ErrType initialize();
    ErrType destroy();

    ErrType allocate(ID3D12Resource** pResource, U64 szBytes, U64 alignment);

    ErrType free(ID3D12Resource* pResource);

    ErrType collectGarbage();
    
private:
    ID3D12Heap* m_pHeap;
    Allocator* m_pAllocator;

    // garbage resources to clean up after use.
    std::vector<ID3D12Resource*> m_garbageToClean;
};
} // Recluse