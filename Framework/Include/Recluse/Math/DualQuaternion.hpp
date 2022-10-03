//
#pragma once

#include "Recluse/Math/Quaternion.hpp"

namespace Recluse {
namespace Math {
struct R_PUBLIC_API DualQuaternion
{
    Quaternion real;    // Real part.
    Quaternion dual;    // Dual part.

    DualQuaternion
            (
                const Quaternion& r = Quaternion(), 
                const Quaternion& d = Quaternion()
            )
        : real(r)
        , dual(d)
    {
    }

    inline DualQuaternion operator+(const DualQuaternion& rh)
    {
        return DualQuaternion(real + rh.real, dual + rh.dual);
    }
    
};
} // Math
} // Recluse