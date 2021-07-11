
#include <iostream>
#include "Core/Messaging.hpp"
#include "Core/RealtimeTick.hpp"
#include "Core/Messaging.hpp"

using namespace Recluse;

int main()
{
    Log::initializeLoggingSystem();

    // Initialize the realtime tick.
    RealtimeTick::initialize();

    // initialize our max time.
    F32 seconds     = 0.f;
    F32 maxTimeS    = 60.0f;

    while(1) {

        // Get the current tick.
        RealtimeTick tick   = RealtimeTick::getTick();
        seconds            += tick.getDeltaTimeS();

        R_VERBOSE("TIMING", "Current Time: %f, Delta Time: %f Seconds: %f", tick.getCurrentTimeS(), tick.getDeltaTimeS(), seconds);

        if (seconds >= maxTimeS) {
            break;
        }

    }

    Log::destroyLoggingSystem();

    return 0;
}