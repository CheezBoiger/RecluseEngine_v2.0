//
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Namespace.hpp"
#include "Recluse/Memory/MemoryPool.hpp"


void* operator new (size_t sizeBytes, Recluse::Allocator* alloc, Recluse::Allocation* pOutput)
{
    R_ASSERT(alloc != NULL);

    Recluse::Allocation allocationOutput    = { 0ull, 0ull };
    Recluse::ResultCode err                    = alloc->allocate(&allocationOutput, sizeBytes, Recluse::pointerSizeBytes());

    if (pOutput)
    {
        pOutput->baseAddress    = allocationOutput.baseAddress;
        pOutput->sizeBytes      = allocationOutput.sizeBytes;
    }

    R_ASSERT(err == Recluse::RecluseResult_Ok);

    return (void*)allocationOutput.baseAddress;
}


void* operator new (size_t sizeBytes, Recluse::Allocator* alloc)
{
    R_ASSERT(alloc != NULL);

    Recluse::Allocation allocationOutput    = { 0ull, 0ull };
    Recluse::ResultCode err                    = alloc->allocate(&allocationOutput, sizeBytes, Recluse::pointerSizeBytes());

    R_ASSERT(err == Recluse::RecluseResult_Ok);

    return (void*)allocationOutput.baseAddress;
}


void operator delete(void* ptr, Recluse::Allocator* alloc)
{
    R_ASSERT(alloc != NULL);
    // In most cases we can get away with just knowing the pointer address.
    // Most allocators will accept just this.
    Recluse::Allocation allocation;

    allocation.baseAddress  = (Recluse::UPtr)ptr;
    allocation.sizeBytes    = 0ull;

    Recluse::ResultCode err = alloc->free(&allocation);

    R_ASSERT(err == Recluse::RecluseResult_Ok);
}
