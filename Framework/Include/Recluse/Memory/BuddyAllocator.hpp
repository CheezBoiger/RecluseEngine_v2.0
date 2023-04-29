//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Memory/Allocator.hpp"

#include <vector>
#include <map>

namespace Recluse {


struct BuddyBlock 
{
    U64     memSzBytes;     // Likely sizes of 64 KB
    U64     offsetBytes;    // Offset of the buddy block in virtual memory.
    U64     blockId;        // block id.
};

// Buddy allocator implementation.
//
class R_PUBLIC_API BuddyAllocator : public Allocator 
{
private:
    typedef Allocation BlockAllocation;

    ResultCode onInitialize() override;
    ResultCode onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override;
    ResultCode onFree(Allocation* pOutput) override;
    ResultCode onReset() override;
    ResultCode onCleanUp() override;

    inline BlockAllocation makeBlockAllocation(U64 blockAddress, U64 blockSzBytes)
    {
        return { blockAddress, blockSzBytes };
    }

    std::vector<std::vector<BuddyBlock>>        m_freeList;
    std::map<SizeT, BlockAllocation>            m_allocatedBlocks;
    U32                                         m_maxOrder;
};
} // Recluse