
#include <iostream>

#include "Recluse/Messaging.hpp"
#include "Recluse/Time.hpp"

using namespace Recluse;

int main()
{
    Log::initializeLoggingSystem();

    // Initialize the realtime tick.
    RealtimeTick::initializeWatch(1ull, 0);

    // initialize our max time.
    F32 seconds     = 0.f;
    F32 maxTimeS    = 60.0f;

    while(1) {
        RealtimeTick::updateWatch(1ull, 0);

        // Get the current tick.
        RealtimeTick tick   = RealtimeTick::getTick(0);
        seconds            += tick.delta();

        R_VERBOSE("TIMING", "Current Time: %f, Delta Time: %f Seconds: %f", tick.getCurrentTimeS(), tick.delta(), seconds);

        if (seconds >= maxTimeS) {
            break;
        }

    }

    Log::destroyLoggingSystem();

    return 0;
}