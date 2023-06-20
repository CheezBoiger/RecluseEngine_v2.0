//
#include "Recluse/Memory/BuddyAllocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Messaging.hpp"

#include <math.h>

namespace Recluse {


ResultCode BuddyAllocator::onInitialize()
{
    U64 totalSzBytes    = getTotalSizeBytes();
    UPtr baseAddr    = getBaseAddr();

    // Size must be power of 2.
    R_ASSERT(Math::isPowerOf2(totalSzBytes));
    
    U32 nthBit  = (U32)ceil(log2(totalSzBytes));  

    m_maxOrder  = nthBit + 1;

    m_freeList.resize(m_maxOrder);

    BuddyBlock block    = { };
    block.blockId       = nthBit;
    block.offsetBytes   = baseAddr;
    block.memSzBytes    = totalSzBytes;

    // Store big block as initialized to the whole memory region.
    m_freeList[nthBit].push_back(block);

    return RecluseResult_Ok;
}


ResultCode BuddyAllocator::onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment)
{
    R_ASSERT(Math::isPowerOf2(alignment));
    UPtr baseAddr    = getBaseAddr();
    U64 neededSzBytes   = requestSz + (alignment - 1);
    
    U32 nthBit          = (U32)ceil(log2(neededSzBytes));
    
    if (m_freeList[nthBit].size() > 0) 
    {
        BuddyBlock block = m_freeList[nthBit][0];
        m_freeList[nthBit].erase(m_freeList[nthBit].begin());
        U64 blockAddress                        = baseAddr + block.offsetBytes;
        pOutput->baseAddress                    = align(baseAddr + block.offsetBytes, alignment);
        pOutput->sizeBytes                      = block.memSzBytes;
        m_allocatedBlocks[pOutput->baseAddress] = makeBlockAllocation(blockAddress, block.memSzBytes);
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
            return RecluseResult_OutOfMemory;   
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
                blockLeft.memSzBytes    = (block.memSzBytes >> 1ull);
                
                blockRight.offsetBytes  = blockLeft.offsetBytes + blockLeft.memSzBytes;
                blockRight.memSzBytes   = (block.memSzBytes >> 1ull);

                m_freeList[i].push_back(blockLeft);
                m_freeList[i].push_back(blockRight);

                block = m_freeList[i][0];
                m_freeList[i].erase(m_freeList[i].begin());
            }
        
            // Allocate the buddy block. Use the aligned address as the key to the mapped buddy block.
            U64 blockAddress                        = baseAddr + block.offsetBytes;
            pOutput->baseAddress                    = align(blockAddress, alignment);
            pOutput->sizeBytes                      = block.memSzBytes;
            m_allocatedBlocks[pOutput->baseAddress] = makeBlockAllocation(blockAddress, block.memSzBytes);
        }
    }

    return RecluseResult_Ok;
}


ResultCode BuddyAllocator::onFree(Allocation* pOutput)
{
    if (m_allocatedBlocks.find(pOutput->baseAddress) == m_allocatedBlocks.end()) 
    {
        return RecluseResult_Failed;
    
    }

    // Get the block allocation using the aligned address.
    const BlockAllocation& blockAllocation = m_allocatedBlocks[pOutput->baseAddress];    

    UPtr baseAddr    = getBaseAddr();
    //U64 szBytes         = pOutput->sizeBytes;
    U32 nthBit          = (U32)ceil(log2(blockAllocation.sizeBytes));
    U32 buddyNumber     = (U32)(blockAllocation.baseAddress / blockAllocation.sizeBytes);
    SizeT buddyAddr     = 0;
    U64 allocOffset     = blockAllocation.baseAddress - baseAddr;

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
            if (Math::isEven(buddyNumber)) 
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

    // Erase the block after, using the aligned address.
    pOutput->sizeBytes = blockAllocation.sizeBytes;
    m_allocatedBlocks.erase(pOutput->baseAddress);
    return RecluseResult_Ok;
}


ResultCode BuddyAllocator::onReset()
{
    return RecluseResult_NoImpl;
}


ResultCode BuddyAllocator::onCleanUp()
{
    // Wipe out everything.
    m_freeList.clear();
    m_allocatedBlocks.clear();

    return RecluseResult_Ok;
}
} // Recluse