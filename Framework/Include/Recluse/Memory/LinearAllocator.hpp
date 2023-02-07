//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


// Stack allocator, or linear allocator, which handles 
// temporary data to be used briefly.
class R_PUBLIC_API LinearAllocator : public Allocator 
{
public:
    LinearAllocator()
        : m_top(0ull) { }

    ErrType onInitialize() override 
    {
        m_top = (PtrType)getBaseAddr();
        return RecluseResult_Ok;
    }

    ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override 
    {
        PtrType neededSzBytes   = requestSz + alignment;
        U64 totalSzBytes        = getTotalSizeBytes();
        PtrType szAddr          = getBaseAddr() + totalSzBytes;
        PtrType endAddr         = m_top + neededSzBytes;
    
        if (endAddr >= szAddr) 
        {
            return RecluseResult_OutOfMemory;
        }

        pOutput->baseAddress        = align(m_top, alignment);
        pOutput->sizeBytes          = neededSzBytes;
        m_top                       = endAddr;

        return RecluseResult_Ok;
    }

    PtrType getTop() const 
    {
        return m_top;
    }

    ErrType onReset() override 
    { 
        m_top = (PtrType)getBaseAddr();
        return RecluseResult_Ok;
    }

    ErrType onCleanUp() override 
    {
        return RecluseResult_Ok;
    }

    ErrType onFree(Allocation* pOutput) override
    {
        return RecluseResult_Ok;
    }

private:
    PtrType m_top;
};
} // Recluse