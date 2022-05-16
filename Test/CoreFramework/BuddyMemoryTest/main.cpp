
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
    R_DEBUG("Core", "Finished: %f secs", tick.getDeltaTimeS());
}

int main()
{
    Log::initializeLoggingSystem();

    // Initialize the realtime tick.
    RealtimeTick::initializeWatch(1ull, 0);

    R_DEBUG("Core", "Initialize buddy memory allocator.");
    BuddyAllocator* pAllocator = new BuddyAllocator();
    pAllocator->initialize(0, R_ALLOC_MASK(R_1MB, 4));
    Allocation alloc    = { };
    Allocation alloc2   = { };

    R_DEBUG("Core", "Allocating buddy block.");

    pAllocator->allocate(&alloc, 150, 4);
    logTick();
    pAllocator->allocate(&alloc2, 512, 4);
    logTick();

    R_TRACE("Core", "Allocation: \t\t%llu bytes\n\t\toffset:\t\t%llu", alloc.sizeBytes, alloc.baseAddress);
    R_TRACE("Core", "Allocation2: \t%llu bytes\n\t\toffset:\t\t%llu", alloc2.sizeBytes, alloc2.baseAddress);

    pAllocator->free(&alloc);
    logTick();

    pAllocator->free(&alloc2);
    logTick();

    pAllocator->cleanUp();
    delete pAllocator;

    R_DEBUG("Core", "Finished!");

    Log::destroyLoggingSystem();
    return 0;
}