//
#pragma once

#include "Recluse/Math/Quaternion.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {
namespace Math {
struct RecluseFramework_PUBLIC_API DualQuaternion
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

    inline DualQuaternion operator+(const DualQuaternion& rh) const
    {
        return DualQuaternion(real + rh.real, dual + rh.dual);
    }

    inline DualQuaternion operator-(const DualQuaternion& rh) const
    {
        return DualQuaternion(real - rh.real, dual - rh.dual);
    }
    
};
} // Math
} // Recluse