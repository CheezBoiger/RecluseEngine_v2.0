//
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/MemoryScan.hpp"

#include <stdlib.h>

namespace Recluse {


MemoryPool::MemoryPool(U64 szBytes, U64 pageSz)
    : m_baseAddr(0ull)
    , m_pageSzBytes(pageSz)
    , m_totalSzBytes(0ull)
    , m_pScanStart(nullptr)
    , m_isMalloc(true)
{
    U64 allocationSizeBytes = szBytes;
    if (pageSz <= 4096ull)
    {
        
    }

    m_baseAddr = (PtrType)malloc(allocationSizeBytes);
    m_pageSzBytes = pageSz;
    m_totalSzBytes = allocationSizeBytes;
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
    // Broadcast to observer scanners.
    MemScanNodes* trav = m_pScanStart;
    while (trav)
    {
        trav->pScanner->scanMemoryLeaks(this);
        trav = trav->next;
    }

    if (m_baseAddr && m_isMalloc)
    {
        free((void*)m_baseAddr);
    }
}
} // Recluse