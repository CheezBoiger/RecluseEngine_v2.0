//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include <list>

namespace Recluse {


class FreeListAllocator : public Allocator 
{
private:

    struct MemBlock 
    {
        U64         addressOffsetBytes;
        U64         blockSizeBytes;

        MemBlock*   pPrev;
        MemBlock*   pNext;
    };

public:

    FreeListAllocator()
        : m_memBlockStart(nullptr) { }

    virtual ~FreeListAllocator() 
    {
        m_memBlocks.clear();
    }

    virtual ErrType onInitialize() override;
    virtual ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override;
    virtual ErrType onFree(Allocation* pOutput) override;
    
    virtual ErrType onReset() override;
    virtual ErrType onCleanUp() override;

private:
    
    MemBlock*           m_memBlockStart;
    std::list<MemBlock> m_memBlocks;
};
} // Recluse