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
        return R_RESULT_OK;
    }

    ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override 
    {
        PtrType neededSzBytes   = R_ALLOC_MASK(requestSz, alignment);
        U64 totalSzBytes        = getTotalSizeBytes();
        PtrType szAddr          = getBaseAddr() + totalSzBytes;
        PtrType endAddr         = m_top + neededSzBytes;
    
        if (endAddr >= szAddr) 
        {
            return R_RESULT_OUT_OF_MEMORY;
        }

        pOutput->baseAddress        = m_top;
        pOutput->sizeBytes          = neededSzBytes;
        m_top                       = endAddr;

        return R_RESULT_OK;
    }

    PtrType getTop() const 
    {
        return m_top;
    }

    ErrType onReset() override 
    { 
        m_top = (PtrType)getBaseAddr();
        return R_RESULT_OK;
    }

    ErrType onCleanUp() override 
    {
        return R_RESULT_OK;
    }

    ErrType onFree(Allocation* pOutput) override
    {
        return R_RESULT_OK;
    }

private:
    PtrType m_top;
};
} // Recluse