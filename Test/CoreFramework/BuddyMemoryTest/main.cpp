
#include <iostream>
#include "Recluse/Memory/BuddyAllocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Messaging.hpp"

using namespace Recluse;

inline void logTick()
{
    RealtimeTick::updateWatch(1ull, 0);
    RealtimeTick tick = RealtimeTick::getTick(0);
    R_DEBUG("Core", "Finished: %f secs", tick.delta());
}

int main()
{
    Log::initializeLoggingSystem();

    // Initialize the realtime tick.
    RealtimeTick::initializeWatch(1ull, 0);

    R_DEBUG("Core", "Initialize buddy memory allocator.");
    BuddyAllocator* pAllocator = new BuddyAllocator();
    pAllocator->initialize(0, align(R_1MB, 4));

    R_DEBUG("Core", "Allocating buddy block.");

    UPtr alloc = pAllocator->allocate(257, 4);
    logTick();
    UPtr alloc2 = pAllocator->allocate(512, 4);
    logTick();

    R_TRACE("Core", "Allocation: \t\t%llu bytes\n\t\toffset:\t\t%llu", 257, alloc);
    R_TRACE("Core", "Allocation2: \t%llu bytes\n\t\toffset:\t\t%llu", 512, alloc2);
    R_TRACE("Core", "Allocator: allocations: %d, allocated mem: %d bytes", pAllocator->getTotalAllocations(), pAllocator->getUsedSizeBytes());

    pAllocator->free(alloc);
    logTick();

    pAllocator->free(alloc2);
    logTick();
    R_TRACE("Core", "Allocator: allocations: %d, allocated mem: %d bytes", pAllocator->getTotalAllocations(), pAllocator->getUsedSizeBytes());

    pAllocator->cleanUp();
    delete pAllocator;

    R_DEBUG("Core", "Finished!");

    Log::destroyLoggingSystem();
    return 0;
}