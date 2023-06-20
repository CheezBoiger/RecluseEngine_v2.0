//
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Namespace.hpp"
#include "Recluse/Memory/MemoryPool.hpp"


void* operator new (size_t sizeBytes, Recluse::Allocator* alloc)
{
    R_ASSERT(alloc != NULL);
    return (void*)alloc->allocate(sizeBytes, Recluse::pointerSizeBytes());
}


void operator delete(void* ptr, Recluse::Allocator* alloc)
{
    R_ASSERT(alloc != NULL);

    alloc->free((Recluse::UPtr)ptr);

    R_DEBUG_WRAP(Recluse::ResultCode err = alloc->getLastError());
    R_DEBUG_WRAP(R_ASSERT(err == Recluse::RecluseResult_Ok));
}
