
#include <iostream>
#include "Core/Messaging.hpp"
#include "Core/RealtimeTick.hpp"

using namespace Recluse;

int main()
{
    // Initialize the realtime tick.
    RealtimeTick::initialize();

    // initialize our max time.
    F32 seconds     = 0.f;
    F32 maxTimeS    = 5.0f;

    while(1) {

        // Get the current tick.
        RealtimeTick tick   = RealtimeTick::GetTick();
        seconds            += tick.getDeltaTimeS();

        printf("Current Time: %f, Delta Time: %f Seconds: %f\n", tick.getCurrentTimeS(), tick.getDeltaTimeS(), seconds);

        if (seconds >= maxTimeS) {
            break;
        }

    }

    return 0;
}