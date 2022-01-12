//
#include "Recluse/Memory/BuddyAllocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Messaging.hpp"

#include <math.h>

namespace Recluse {


ErrType BuddyAllocator::onInitialize()
{
    U64 totalSzBytes    = getTotalSizeBytes();
    PtrType baseAddr    = getBaseAddr();

    // Size must be power of 2.
    R_ASSERT((totalSzBytes & (totalSzBytes - 1)) == 0);
    
    U32 nthBit  = (U32)ceil(log2(totalSzBytes));  

    m_maxOrder  = nthBit + 1;

    m_freeList.resize(m_maxOrder);

    BuddyBlock block    = { };
    block.blockId       = nthBit;
    block.offsetBytes   = baseAddr;
    block.memSzBytes    = totalSzBytes;

    // Store big block as initialized to the whole memory region.
    m_freeList[nthBit].push_back(block);

    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment)
{
    PtrType baseAddr    = getBaseAddr();
    U64 alignedSzBytes  = R_ALLOC_MASK(requestSz, alignment);
    
    U32 nthBit          = (U32)ceil(log2(alignedSzBytes));
    
    if (m_freeList[nthBit].size() > 0) 
    {
    
        BuddyBlock block = m_freeList[nthBit][0];
        m_freeList[nthBit].erase(m_freeList[nthBit].begin());

        pOutput->baseAddress                    = baseAddr + block.offsetBytes;
        pOutput->sizeBytes                      = alignedSzBytes;
        m_allocatedBlocks[pOutput->baseAddress] = block.memSzBytes;
    
    } 
    else 
    {
    
        U32 i = 0;
        for (i = nthBit + 1; i < m_maxOrder; ++i) 
        {
            if (m_freeList[i].size() != 0) 
            {
                break;
            }

        }

        if (i == m_maxOrder) 
        {
            return REC_RESULT_OUT_OF_MEMORY;   
        } 
        else 
        {
        
            BuddyBlock block    = { };
            block               = m_freeList[i][0];

            // Split block into halves.
            m_freeList[i].erase(m_freeList[i].begin());
            i--;

            for (; i >= nthBit; --i) 
            {
                // split to two blocks and push back.
                BuddyBlock blockLeft;
                BuddyBlock blockRight;
                
                blockLeft.offsetBytes   = block.offsetBytes;
                blockLeft.memSzBytes    = block.memSzBytes / 2;
                
                blockRight.offsetBytes  = blockLeft.offsetBytes + blockLeft.memSzBytes;
                blockRight.memSzBytes   = block.memSzBytes / 2;

                m_freeList[i].push_back(blockLeft);
                m_freeList[i].push_back(blockRight);

                block = m_freeList[i][0];
                m_freeList[i].erase(m_freeList[i].begin());
            }
        
            pOutput->baseAddress                    = baseAddr + block.offsetBytes;
            pOutput->sizeBytes                      = alignedSzBytes;
            m_allocatedBlocks[pOutput->baseAddress] = block.memSzBytes;
        }
    }

    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onFree(Allocation* pOutput)
{
    if (m_allocatedBlocks.find(pOutput->baseAddress) == m_allocatedBlocks.end()) 
    {
        return REC_RESULT_FAILED;
    
    }

    PtrType baseAddr    = getBaseAddr();
    U64 szBytes         = pOutput->sizeBytes;
    U32 nthBit          = (U32)ceil(log2(szBytes));
    U32 buddyNumber     = (U32)(pOutput->baseAddress / m_allocatedBlocks[pOutput->baseAddress]);
    SizeT buddyAddr     = 0;
    U64 allocOffset     = pOutput->baseAddress - baseAddr;

    // memSzBytes is always the power of our N-th value.
    BuddyBlock block    = { };
    block.offsetBytes   = allocOffset;
    block.memSzBytes    = (1ull << nthBit);

    m_freeList[nthBit].push_back(block);

    buddyAddr = (buddyNumber & 1)
        ? (allocOffset - (1ull << nthBit)) 
        : (allocOffset + (1ull << nthBit));

    //
    for (U32 i = 0; i < m_freeList[nthBit].size(); ++i) 
    {
        // Merge blocks together, if applicable.
        if (m_freeList[nthBit][i].offsetBytes == buddyAddr) 
        {
            // Check if even, merge with alloc offset. Otherwise, use buddyaddr
            if (!(buddyNumber & 1)) 
            {
                BuddyBlock newBlock     = { };
                newBlock.offsetBytes    = allocOffset;
                newBlock.memSzBytes     = 2ull * (1ull << nthBit);
                m_freeList[nthBit + 1].push_back(newBlock);
             
            } 
            else 
            {
                BuddyBlock newBlock     = { };
                newBlock.offsetBytes    = buddyAddr;
                newBlock.memSzBytes     = 2ull * (1ull << nthBit);
                m_freeList[nthBit + 1].push_back(newBlock);
            
            }

            m_freeList[nthBit].erase(m_freeList[nthBit].begin() + i);
            m_freeList[nthBit].erase(m_freeList[nthBit].begin() + m_freeList[nthBit].size() - 1);

            break;
        }
    }

    // Erase the block after.
    m_allocatedBlocks.erase(pOutput->baseAddress);

    return REC_RESULT_OK;
}


ErrType BuddyAllocator::onReset()
{
    return REC_RESULT_NOT_IMPLEMENTED;
}


ErrType BuddyAllocator::onCleanUp()
{
    // Wipe out everything.
    m_freeList.clear();
    m_allocatedBlocks.clear();

    return REC_RESULT_OK;
}
} // Recluse