//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

// Realtime Tick object. Handles Obtaining the 
// tick from the operating system clock.
class RealtimeTick 
{
public:

    // Get the realtime tick.
    static R_PUBLIC_API RealtimeTick getTick(U32 watchType);

    // Get the current time in sec.
    F32 getCurrentTimeS() const { return m_currentTimeS; }

    // Get the delta time in sec.
    F32 getDeltaTimeS() const { return m_deltaTimeS; }
    
    // Initialize the realtime tick manager. Should be 
    // called first, before calling getTick().
    static R_PUBLIC_API void initializeWatch(U32 watchType);

private:
    // Realtime tick initializer. Be sure to call this once and reference
    // the object across all objects!
    RealtimeTick(U32 watchType);

    // Current time in seconds.
    F32 m_currentTimeS;

    // Delta time in seconds.
    F32 m_deltaTimeS;
};
} // Recluse