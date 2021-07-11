
#include <iostream>
#include "Core/Memory/BuddyAllocator.hpp"
#include "Core/Messaging.hpp"
#include "Core/RealtimeTick.hpp"
#include "Core/Messaging.hpp"

using namespace Recluse;

inline void logTick()
{
    RealtimeTick tick = RealtimeTick::getTick();
    R_DEBUG("Core", "Finished: %f secs", tick.getDeltaTimeS());
}

int main()
{
    Log::initializeLoggingSystem();

    // Initialize the realtime tick.
    RealtimeTick::initialize();

    R_DEBUG("Core", "Initialize buddy memory allocator.");
    BuddyAllocator* pAllocator = new BuddyAllocator();
    pAllocator->initialize(0, RECLUSE_ALLOC_MASK(RECLUSE_1MB, 4));
    Allocation alloc    = { };
    Allocation alloc2   = { };

    R_DEBUG("Core", "Allocating buddy block.");

    pAllocator->allocate(&alloc, 150, 4);
    logTick();
    pAllocator->allocate(&alloc2, 512, 4);
    logTick();

    R_TRACE("Core", "Allocation: \t\t%llu bytes\n\t\toffset:\t\t%llu", alloc.sizeBytes, alloc.ptr);
    R_TRACE("Core", "Allocation2: \t%llu bytes\n\t\toffset:\t\t%llu", alloc2.sizeBytes, alloc2.ptr);

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