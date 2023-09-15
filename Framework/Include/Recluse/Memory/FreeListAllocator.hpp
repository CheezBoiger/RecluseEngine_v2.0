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
    };

public:

    FreeListAllocator()
        : m_memBlockStart(nullptr) { }

    virtual ~FreeListAllocator() 
    {
        m_memBlocks.clear();
    }

    virtual ResultCode onInitialize() override;
    virtual ResultCode onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override;
    virtual ResultCode onFree(Allocation* pOutput) override;
    
    virtual ResultCode onReset() override;
    virtual ResultCode onCleanUp() override;

private:
    
    MemBlock*           m_memBlockStart;
    std::list<MemBlock> m_memBlocks;
};
} // Recluse