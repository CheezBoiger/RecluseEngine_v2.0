//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Math/Vector4.hpp"


namespace Recluse {


struct Quaternion {
    F32 x, y, z, w;

    Quaternion(F32 x, F32 y, F32 z, F32 w)
        : x(x), y(y), z(z), w(w) { }
};
} // Recluse