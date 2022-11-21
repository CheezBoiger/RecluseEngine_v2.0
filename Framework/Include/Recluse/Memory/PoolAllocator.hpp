//

#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


class R_PUBLIC_API PoolAllocator : public Allocator
{
public:
    ErrType onInitialize() override
    {
        return RecluseResult_NoImpl;
    }

    ErrType onCleanUp() override
    {
        return RecluseResult_NoImpl;
    }
};
} // Recluse