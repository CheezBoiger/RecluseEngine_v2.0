//
#ifndef RECLUSE_MEMORY_POOL_HPP
#define RECLUSE_MEMORY_POOL_HPP

#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


template<typename Type, typename ...Arguments>
static Type* rlsMalloc(Arguments... args)
{
    return new Type(args...);
}


template<typename Type>
static Type* rlsMalloc(U64 count)
{
    return new Type[count];
}


template<typename Type>
static void rlsFree(Type* ptr)
{
    delete ptr;
}


template<typename Type>
static void rlsFreeArray(Type* ptr)
{
    delete[] ptr;
}


class MemoryScanner;


// Memory Pool consists of an allocated space that will be used for suballocations, or committed space.
// This arena is intended to be used with Allocators.
class R_PUBLIC_API MemoryPool 
{
public:
    // Memory pool allocation construction.
    MemoryPool(U64 szBytes = 0ull, U64 pageSize = 4096ull);

    // Memory must be malloc'ed or new'ed. Can not be stack local memory!
    MemoryPool(void* ptr, U64 szBytes, U64 pagSz = 4096ull);

    ~MemoryPool();
        
    // Get the starting address of the memory pool.
    UPtr                getBaseAddress() const { return m_baseAddr; }

    // Get the total size of the pool, in bytes.
    U64                 getTotalSizeBytes() const { return m_totalSzBytes; }

    // Get the page size used for the pool, in bytes.
    U64                 getPageSzBytes() const { return m_pageSzBytes; }

    inline void*        getRawAddressAt(U64 sizeBytes) { return reinterpret_cast<void*>(m_baseAddr + sizeBytes); }
    inline UPtr         getPtrAddressAt(U64 sizeBytes) { return (m_baseAddr + sizeBytes); }

    // Add a memory leak scanner to the pool, to ensure there are no memory leaks.
    void                addScanner(MemoryScanner* scanner);

    // Pre allocates the memory pool.
    void                preAllocate(U64 szBytes, U64 pageSize = 4096ull);

    // Clear the pool, wipe out the state and set to default value.
    // This does not free the pool memory!
    void                clear(U32 defaultValue = 0);

    // Free the pool memory! This will delete the memory!
    void                release();

    // Check if the memory pool is allocated, otherwise return false if pool is not.
    Bool                isAllocated() { return !!m_totalSzBytes; }

    // Copy contents from source memory pool, to destination memory pool.
    static void         copy(MemoryPool* dst, U64 dstOffset, MemoryPool* src, U64 srcOffset, U64 sizeBytes);

private:

    UPtr    m_baseAddr;
    U64     m_totalSzBytes;
    U64     m_pageSzBytes;
    B32     m_isMalloc;

    struct MemScanNodes 
    {
        MemoryScanner*          pScanner;
        struct MemScanNodes*    next;

        MemScanNodes(MemoryScanner* scanner = nullptr, struct MemScanNodes* nextNode = nullptr)
            : pScanner(scanner)
            , next(nextNode)
        { }
    } *m_pScanStart;
};


// Name aliases
typedef MemoryPool MemoryArena;
typedef MemoryPool ScratchMem;
typedef ScratchMem ScratchMemory;

} // Recluse
#endif // RECLUSE_MEMORY_POOL_HPP