//
#ifndef RECLUSE_MEMORY_POOL_HPP
#define RECLUSE_MEMORY_POOL_HPP

#pragma once

#include "Core/Types.hpp"

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


class MemoryPool {
public:
    MemoryPool(U64 szBytes, U64 pageSize = 4096ull);

    // Memory must be malloc'ed or new'ed. Can not be stack local memory!
    MemoryPool(void* ptr, U64 szBytes, U64 pagSz = 4096ull);

    ~MemoryPool();
        
    PtrType getBaseAddress() const { return m_baseAddr; }

    U64 getTotalSizeBytes() const { return m_totalSzBytes; }

    U64 getPageSzBytes() const { return m_pageSzBytes; }

    void addScanner(MemoryScanner* scanner);

private:
    PtrType m_baseAddr;
    U64     m_totalSzBytes;
    U64     m_pageSzBytes;
    B32     m_isMalloc;

    struct MemScanNodes {
        MemoryScanner* pScanner;
        struct MemScanNodes* next;

        MemScanNodes(MemoryScanner* scanner = nullptr, struct MemScanNodes* nextNode = nullptr)
            : pScanner(scanner)
            , next(nextNode)
        { }
    } *m_pScanStart;
};
} // Recluse
#endif // RECLUSE_MEMORY_POOL_HPP