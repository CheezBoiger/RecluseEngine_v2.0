//
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/MemoryScan.hpp"

#include "Recluse/Messaging.hpp"

#include <stdlib.h>

namespace Recluse {


MemoryPool::MemoryPool(U64 szBytes, U64 pageSz)
    : m_baseAddr(0ull)
    , m_pageSzBytes(pageSz)
    , m_totalSzBytes(0ull)
    , m_pScanStart(nullptr)
    , m_isMalloc(false)     // Start with no malloc, if we are just empty initializing this pool.
{
    preAllocate(szBytes, pageSz);
}


MemoryPool::MemoryPool(void* ptr, U64 szBytes, U64 pageSz)
    : m_baseAddr(0ull)
    , m_pageSzBytes(pageSz)
    , m_totalSzBytes(0ull)
    , m_pScanStart(nullptr)
    , m_isMalloc(false)
{
    m_baseAddr = (PtrType)ptr;
    m_pageSzBytes = pageSz;
    m_totalSzBytes = szBytes;
}


void MemoryPool::addScanner(MemoryScanner* scanner)
{
    MemScanNodes* trav = m_pScanStart;
    if (!m_pScanStart) 
    {
        m_pScanStart = new MemScanNodes(scanner);
        return;
    }

    MemScanNodes* node = new MemScanNodes(scanner);
    
    while (trav->next) 
    {
        trav = trav->next;
    }

    trav->next = node;
}


MemoryPool::~MemoryPool()
{
    release();
}


void MemoryPool::preAllocate(U64 szBytes, U64 pageSz)
{
    U64 allocationSizeBytes = szBytes;

    if (szBytes == 0ull || isAllocated())
    {
        // We don't have requested size bytes to allocate. This pre-allocation will be ignored.
        R_WARN
            (
                "MemoryPool", 
                "Memory pool is either already allocated, or 0 size bytes was passed to preAllocation."
                " Be sure to either call release(), or check if the correct behavior was expected."
            );
        return;
    }

    if (pageSz <= 4096ull)
    {

    }

    m_baseAddr      = (PtrType)malloc(allocationSizeBytes);
    m_pageSzBytes   = pageSz;
    m_totalSzBytes  = allocationSizeBytes;
    m_isMalloc      = true;
}


void MemoryPool::clear(U32 defaultValue)
{
    if (m_totalSzBytes == 0ull)
    {
        return;
    }
    // Set the memory pool to a default value, this is our clear!
    memset((void*)m_baseAddr, defaultValue, m_totalSzBytes);
}


void MemoryPool::release()
{
    // Broadcast to observer scanners.
    MemScanNodes* trav = m_pScanStart;

    while (trav)
    {
        trav->pScanner->scanMemoryLeaks(this);
        trav = trav->next;
    }

    if (m_baseAddr && m_isMalloc)
    {
        // Free the base address, and since it is malloc'ed, we need to point the address back to zero, to let the pool know
        // we no longer have memory attached.
        free((void*)m_baseAddr);
        m_baseAddr = 0;
    }
}
} // Recluse