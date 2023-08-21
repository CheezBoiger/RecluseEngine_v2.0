//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


// Stack allocator, or linear allocator, which handles 
// temporary data to be used briefly.
class R_PUBLIC_API LinearAllocator : public Allocator 
{
public:
    LinearAllocator()
        : m_top(0ull) { }

    ResultCode onInitialize() override 
    {
        m_top = (UPtr)getBaseAddr();
        return RecluseResult_Ok;
    }

    ResultCode onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override 
    {
        R_ASSERT(Math::isPowerOf2(alignment));
        UPtr neededSzBytes   = requestSz + alignOffset(m_top, alignment);
        U64 totalSzBytes     = getTotalSizeBytes();
        UPtr szAddr          = getBaseAddr() + totalSzBytes;
        UPtr endAddr         = m_top + neededSzBytes;
    
        if (endAddr >= szAddr) 
        {
            return RecluseResult_OutOfMemory;
        }

        pOutput->baseAddress        = align(m_top, alignment);
        pOutput->sizeBytes          = neededSzBytes;
        m_top                       = endAddr;

        return RecluseResult_Ok;
    }

    UPtr getTop() const 
    {
        return m_top;
    }

    ResultCode onReset() override 
    { 
        m_top = (UPtr)getBaseAddr();
        return RecluseResult_Ok;
    }

    ResultCode onCleanUp() override 
    {
        return RecluseResult_Ok;
    }

    ResultCode onFree(Allocation* pOutput) override
    {
        return RecluseResult_Ok;
    }

private:
    UPtr m_top;
};
} // Recluse