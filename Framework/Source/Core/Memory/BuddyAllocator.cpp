//
#include "Core/Memory/BuddyAllocator.hpp"

#include "Core/Messaging.hpp"

#include <math.h>

namespace Recluse {


ErrType BuddyAllocator::onInitialize()
{
    U64 totalSzBytes    = getTotalSizeBytes();
    PtrType baseAddr    = getBaseAddr();

    // Size must be power of 2.
    R_ASSERT((totalSzBytes & (totalSzBytes - 1)) == 0);
    
    U32 bit     = totalSzBytes ^ (totalSzBytes & (totalSzBytes - 1));
    U32 nthBit  = (U32)log2(bit) + 1;
    
    m_maxOrder = nthBit;

    m_freeList.resize(m_maxOrder);

    BuddyBlock block = { };
    block.blockId = nthBit;
    block.offsetBytes = baseAddr;
    block.memSzBytes  = totalSzBytes;

    // Store big block as initialized to the whole memory region.
    m_freeList[nthBit - 1].push_back(block);

    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment)
{
    PtrType baseAddr    = getBaseAddr();
    U64 alignedSzBytes  = RECLUSE_ALLOC_MASK(requestSz, alignment);
    U32 bit             = alignedSzBytes ^ (alignedSzBytes & (alignedSzBytes - 1));
    U32 nthBit          = (U32)log2(bit) + 1;

    if (m_freeList[nthBit].size() > 0) {
    
        BuddyBlock block = m_freeList[nthBit][0];
        m_freeList[nthBit].erase(m_freeList[nthBit].begin());

        pOutput->ptr        = baseAddr + block.offsetBytes;
        pOutput->sizeBytes  = alignedSzBytes; 

        m_allocatedBlocks[pOutput->ptr] = block.memSzBytes;
    
    } else {
    
        U32 i = 0;
        for (i = nthBit + 1; i < m_maxOrder; ++i) {
        
            if (m_freeList[i].size() != 0) {
                break;
            }

        }

        if (i == m_maxOrder) {
        
            return REC_RESULT_OUT_OF_MEMORY;
            
        } else {
        
            BuddyBlock block = { };
            block = m_freeList[i][0];
            // Split block into halves.
            m_freeList[i].erase(m_freeList[i].begin());
            i--;
            for (; i >= nthBit; --i) {

                // split to two blocks and push back.
                BuddyBlock blockLeft;
                BuddyBlock blockRight;
                
                blockLeft.offsetBytes = block.offsetBytes;
                blockLeft.memSzBytes = block.memSzBytes / 2;
                
                blockRight.offsetBytes = blockLeft.offsetBytes + blockLeft.memSzBytes;
                blockRight.memSzBytes = block.memSzBytes / 2;

                m_freeList[i].push_back(blockLeft);
                m_freeList[i].push_back(blockRight);

                block = m_freeList[i][0];
                m_freeList[i].erase(m_freeList[i].begin());

            }
        
            pOutput->ptr = baseAddr + block.offsetBytes;
            pOutput->sizeBytes = alignedSzBytes;

            m_allocatedBlocks[pOutput->ptr] = block.memSzBytes;
        }
    
    }
    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onFree(Allocation* pOutput)
{
    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onReset()
{
    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onCleanUp()
{
    return REC_RESULT_OK;
}
} // Recluse