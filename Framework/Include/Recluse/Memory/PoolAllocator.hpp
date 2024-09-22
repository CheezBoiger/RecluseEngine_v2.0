//

#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


class RecluseFramework_PUBLIC_API PoolAllocator : public Allocator
{
public:
    ResultCode onInitialize() override
    {
        return RecluseResult_NoImpl;
    }

    ResultCode onCleanUp() override
    {
        return RecluseResult_NoImpl;
    }
};
} // Recluse