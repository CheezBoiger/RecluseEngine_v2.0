//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Time.hpp"


namespace Recluse {

// limiter limits cpu performance in milliseconds.
// This is useful if you want to limit the thread. 
// Keep in mind that this is cpu based, which spinlocks until reaching the desired Ms.
// Passing 0 as your desired, will essentially mean that you have no
// desired speed. This will just pass the current delta.
class Limiter
{
public:
    static F32 limit(F32 desiredMs, U32 clockId, U32 watchType)
    {
        F32 counterMs = 0.f;

        // If we request 0 or less, we just return the current delta.
        if (desiredMs <= 0.f)
        {
            return RealtimeTick::getTick(watchType).delta();
        }

        while (counterMs < desiredMs)
        {
            RealtimeTick::updateWatch(clockId, watchType);
            RealtimeTick tick = RealtimeTick::getTick(watchType);
            F32 deltaMs = tick.delta();
            counterMs += deltaMs;
        }
        return counterMs;
    }
};
} // Recluse