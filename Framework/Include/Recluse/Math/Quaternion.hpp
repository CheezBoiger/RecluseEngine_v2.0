//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Math/Vector4.hpp"


namespace Recluse {


struct Quaternion : public Float4 {

    Quaternion(F32 x, F32 y, F32 z, F32 w)
        : Float4(x, y, z, w) { }
};


Quaternion normalize(const Quaternion& quat);
Quaternion norm(const Quaternion& quat);

} // Recluse