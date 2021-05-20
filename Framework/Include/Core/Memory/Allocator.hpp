// 
#ifndef RECLUSE_ALLOCATOR_HPP
#define RECLUSE_ALLOCATOR_HPP
#pragma once

#include "Core/Types.hpp"

#define RECLUSE_ALLOC_MASK(m, a)    (((m) + ((a)-1)) & ~((a)-1))
#define RECLUSE_1KB                 (1024ULL)
#define RECLUSE_1MB                 (1024ULL * 1024ULL)
#define RECLUSE_1GB                 (1024ULL * 1024ULL * 1024ULL)

namespace Recluse {


class MemoryPool;


//! Recluse allocation struct. Contains info of the suballocation for the requested data.
typedef struct Allocation {
    PtrType ptr;
    U64     sizeBytes;
} *PAllocation, &RAllocation;


//! Recluse Allocator class. This is the abstract class that is used for defining multiple allocation
//! data structures.
class Allocator {
public:
    virtual ~Allocator() { }

    Allocator() : m_totalAllocations(0), m_totalSizeBytes(0), m_usedSizeBytes(0), m_pMemoryPool(nullptr) { }

    //! Allocator mem size and page size (usually 4kb). 
    void initialize(MemoryPool* pPool, U64 sizeBytes) {
        m_totalSizeBytes = sizeBytes;
        m_pMemoryPool = pPool;
        m_usedSizeBytes = 0;
        onInitialize();
    }

    //! Allocation requirements.
    ErrType allocate(Allocation* pOutput, U64 requestSz, U16 alignment) {
        ErrType err = onAllocate(pOutput, requestSz, alignment);
        if (err == 0) {
            m_totalAllocations += 1;
            m_usedSizeBytes += pOutput->sizeBytes;
        }

        return err;
    }

    void free(Allocation* pOutput) {
        ErrType err = onFree(pOutput);
        if (err == 0) {
            m_usedSizeBytes -= pOutput->sizeBytes;
            m_totalAllocations -= 1;
        }
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
        m_pMemoryPool = nullptr;
    }

    U64 getTotalSizeBytes() const { return m_totalSizeBytes; }
    U64 getUsedSizeBytes() const { return m_usedSizeBytes; }
    U64 getTotalAllocations() const { return m_totalAllocations; }

protected:

    virtual ErrType onInitialize() = 0;
    virtual ErrType onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment) = 0;
    virtual ErrType onFree(Allocation* pOutput) = 0;
    virtual ErrType onReset() = 0;
    virtual ErrType onCleanUp() = 0;

    MemoryPool* getMemoryPool() { return m_pMemoryPool; }

private:
    U64 m_totalSizeBytes;
    U64 m_usedSizeBytes;
    U64 m_totalAllocations;
    MemoryPool* m_pMemoryPool;
};
} // Recluse
#endif // RECLUSE_ALLOCATOR_HPP