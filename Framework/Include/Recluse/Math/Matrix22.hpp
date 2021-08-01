//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


struct R_EXPORT Matrix22 {
    struct { F32 m[4]; };

    F32& operator[](U32 i) { return m[i]; }
    F32 operator[](U32 i) const { return m[i]; }

    Matrix22(F32 a00 = 1.0f, F32 a01 = 0.0f,
             F32 a10 = 0.0f, F32 a11 = 1.0f);
};
} // Recluse