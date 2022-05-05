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
        return REC_RESULT_NOT_IMPLEMENTED;
    }

    ErrType onCleanUp() override
    {
        return REC_RESULT_NOT_IMPLEMENTED;
    }
};
} // Recluse