//
#pragma once

#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"


namespace Recluse {


template<U32 sizeBytes, U32 pageSz = 4096u>
class LinearScratchMemory
{
public:
    
    LinearScratchMemory()
    {
        memArena.preAllocate(sizeBytes + sizeof(LinearAllocator), pageSz);
        allocator = new ((void*)memArena.getBaseAddress()) LinearAllocator();
        allocator->initialize(memArena.getPtrAddressAt(sizeof(LinearAllocator)), memArena.getTotalSizeBytes() - sizeof(LinearAllocator));
    }

    ~LinearScratchMemory()
    {
        allocator->cleanUp();
        memArena.clear();
        memArena.release();
    }

    template<typename Type>
    Type* allocate(U32 arrayCount = 1u)
    {
        R_ASSERT(arrayCount > 0);
        return (Type*)allocator->allocate(sizeof(Type) * arrayCount, pointerSizeBytes());
    }

    void free(void* ptr)
    {
        allocator->free((UPtr)ptr);
    }

    void clear()
    {
        allocator->reset();
    }

    U32 getTotalAllocations() const 
    {
        return allocator->getTotalAllocations();
    }

private:
    MemoryArena memArena;
    LinearAllocator* allocator;
};
} // Recluse