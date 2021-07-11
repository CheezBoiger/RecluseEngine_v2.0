//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


class RealtimeTick {
public:

    static R_EXPORT RealtimeTick getTick();

    F32 getCurrentTimeS() const { return m_currentTimeS; }
    F32 getDeltaTimeS() const { return m_deltaTimeS; }
    
    static R_EXPORT void initialize();

private:
    // Realtime tick initializer. Be sure to call this once and reference
    // the object across all objects!
    RealtimeTick();

    F32 m_currentTimeS;
    F32 m_deltaTimeS;
};
} // Recluse