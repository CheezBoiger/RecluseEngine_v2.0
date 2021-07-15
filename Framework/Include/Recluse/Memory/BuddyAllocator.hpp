//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Memory/Allocator.hpp"

#include <vector>
#include <map>

namespace Recluse {


struct BuddyBlock {
    U64 memSzBytes;     // Likely sizes of 64 KB
    U64 offsetBytes;    // Offset of the buddy block in virtual memory.
    U64 blockId;        // block id.
};

// Buddy allocator implementation.
//
class R_EXPORT BuddyAllocator : public Allocator {
private:
    ErrType onInitialize() override;
    ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override;
    ErrType onFree(Allocation* pOutput) override;
    ErrType onReset() override;
    ErrType onCleanUp() override;

    std::vector<std::vector<BuddyBlock>> m_freeList;
    std::map<SizeT, SizeT>               m_allocatedBlocks;
    U32                                  m_maxOrder;
};
} // Recluse