// 
#ifndef RECLUSE_ALLOCATOR_HPP
#define RECLUSE_ALLOCATOR_HPP
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


class MemoryPool;


//! Recluse allocation struct. Contains info of the suballocation for the requested data.
typedef struct Allocation {
    PtrType ptr;
    U64     sizeBytes;
} *PAllocation, &RAllocation;


//! Recluse Allocator class. This is the abstract class that is used for defining multiple allocation
//! data structures.
class R_EXPORT Allocator {
public:
    virtual ~Allocator() { }

    Allocator() : m_totalAllocations(0), m_totalSizeBytes(0), m_usedSizeBytes(0), m_pMemoryBaseAddr(0ull) { }

    //! Allocator mem size and page size (usually 4kb). 
    void initialize(PtrType pBasePtr, U64 sizeBytes) {
        m_totalSizeBytes = sizeBytes;
        m_pMemoryBaseAddr = pBasePtr;
        m_usedSizeBytes = 0;
        onInitialize();
    }

    //! Allocation requirements.
    ErrType allocate(Allocation* pOutput, U64 requestSz, U16 alignment) {
        ErrType err = onAllocate(pOutput, requestSz, alignment);
        if (err == REC_RESULT_OK) {
            m_totalAllocations += 1;
            m_usedSizeBytes += pOutput->sizeBytes;
        }

        return err;
    }

    ErrType free(Allocation* pOutput) {
        ErrType err = onFree(pOutput);
        if (err == REC_RESULT_OK) {
            m_usedSizeBytes -= pOutput->sizeBytes;
            m_totalAllocations -= 1;
        }
    
        return err;
    }

    void reset() {
        onReset();
        m_usedSizeBytes = 0;
        m_totalAllocations = 0;
    }

    void cleanUp() {
        onCleanUp();
        m_totalAllocations = 0;
        m_totalSizeBytes = 0;
        m_usedSizeBytes = 0;
        m_pMemoryBaseAddr = 0ull;
    }

    inline U64 getTotalSizeBytes() const { return m_totalSizeBytes; }
    inline U64 getUsedSizeBytes() const { return m_usedSizeBytes; }
    inline U64 getTotalAllocations() const { return m_totalAllocations; }

    inline PtrType getBaseAddr() { return m_pMemoryBaseAddr; }

protected:

    virtual ErrType onInitialize() = 0;
    virtual ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) = 0;
    virtual ErrType onFree(Allocation* pOutput) = 0;
    virtual ErrType onReset() = 0;
    virtual ErrType onCleanUp() = 0;

private:
    U64 m_totalSizeBytes;
    PtrType m_pMemoryBaseAddr;
    U64 m_usedSizeBytes;
    U64 m_totalAllocations;
};
} // Recluse
#endif // RECLUSE_ALLOCATOR_HPP