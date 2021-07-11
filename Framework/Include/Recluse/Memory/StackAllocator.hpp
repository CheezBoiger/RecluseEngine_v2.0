//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


class StackAllocator : public Allocator {
public:
    StackAllocator()
        : m_top(0ull) { }

    ErrType onInitialize() override {
        m_top = (PtrType)getBaseAddr();
        return REC_RESULT_OK;
    }

    ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) override {
        PtrType neededSzBytes   = R_ALLOC_MASK(requestSz, alignment);
        U64 totalSzBytes        = getTotalSizeBytes();
        PtrType szAddr          = getBaseAddr() + totalSzBytes;
        PtrType endAddr         = m_top + neededSzBytes;
    
        if (endAddr >= szAddr) {
    
            return REC_RESULT_OUT_OF_MEMORY;
    
        }

        pOutput->ptr        = m_top;
        pOutput->sizeBytes  = neededSzBytes;
        m_top               = endAddr;

        return REC_RESULT_OK;
    }

    ErrType onReset() override { 
        m_top = (PtrType)getBaseAddr();
        return REC_RESULT_OK;
    }

    ErrType onCleanUp() override {
        return REC_RESULT_OK;
    }

    ErrType onFree(Allocation* pOutput) {
        return REC_RESULT_OK;
    }

private:
    PtrType m_top;
};
} // Recluse