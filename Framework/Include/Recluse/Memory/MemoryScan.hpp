//
#ifndef RECLUSE_MEMORY_SCAN_HPP
#define RECLUSE_MEMORY_SCAN_HPP
#pragma once

#include "Recluse/Types.hpp"


namespace Recluse {


class MemoryPool;


class MemoryScanner {
public:
    void scanMemoryLeaks(const MemoryPool* pool) { }

    void getMemoryAllocationLocation() { }
};
} // Recluse
#endif // RECLUSE_MEMORY_SCAN_HPP