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
        return R_RESULT_NO_IMPL;
    }

    ErrType onCleanUp() override
    {
        return R_RESULT_NO_IMPL;
    }
};
} // Recluse