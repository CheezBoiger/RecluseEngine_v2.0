//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

class RealtimeStopWatch;

// Realtime Tick object. Handles Obtaining the 
// tick from the operating system clock.
class RealtimeTick 
{
public:

    // Get the realtime tick. Read-only object, so is safe to query.
    static R_PUBLIC_API R_OS_CALL RealtimeTick getTick(U32 watchType);

    // Updates the watch. This must be called per thread loop, and only ONE 
    // thread should be calling this. Do not call on separate threads, otherwise
    // we end up losing the time for that one.
    static R_PUBLIC_API R_OS_CALL void updateWatch(U64 id, U32 watchType);

    // Get the current time in sec.
    F32 getCurrentTimeS() const { return m_currentTimeS; }

    // Get the delta time in sec.
    F32 getDeltaTimeS() const { return m_deltaTimeS; }
    
    // Initialize the realtime tick manager. Should be 
    // called first, before calling getTick(). Requires an ID in order to 
    // know this is the right system that is updating the watch.
    static R_PUBLIC_API R_OS_CALL void initializeWatch(U64 id, U32 watchType);

protected:
    // Realtime tick initializer. Be sure to call this once and reference
    // the object across all objects!
    R_OS_CALL RealtimeTick(U32 watchType);
    RealtimeTick() { }

    // Current time in seconds.
    F32 m_currentTimeS;

    // Delta time in seconds.
    F32 m_deltaTimeS;

    friend class RealtimeStopWatch;
};


// Stopwatch for just querying the time now, and doesn't worry about anything else.
class RealtimeStopWatch
{
public:
    R_OS_CALL RealtimeStopWatch();

    U64 getCurrentTime() const { return m_currentTimeU64; }

    RealtimeStopWatch operator-(const RealtimeStopWatch& rh);

    operator RealtimeTick();

private:
    U64 m_currentTimeU64;
};
} // Recluse