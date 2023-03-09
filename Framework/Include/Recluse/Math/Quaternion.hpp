//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/MathIntrinsics.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {
namespace Math {

struct R_PUBLIC_API Quaternion 
{
    union 
    {
        struct { F32 x, y, z, w; };
        __m128 row;
    };

    Quaternion(F32 x = 0.0f, F32 y = 0.0f, F32 z = 0.0f, F32 w = 1.0f)
        : x(x), y(y), z(z), w(w) { }

    F32& operator[](U32 i) { return (&x)[i]; }
    F32 operator[](U32 i) const { return (&x)[i]; }

    inline Quaternion operator+(const Quaternion& rh) const;
    inline Quaternion operator-(const Quaternion& rh) const;
    inline Quaternion operator*(const Quaternion& rh) const;

    inline Quaternion operator*(F32 scalar) const;
    inline Quaternion operator+(F32 scalar) const;
    inline Quaternion operator-(F32 scalar) const;
    inline Quaternion operator/(F32 scalar) const;

    inline Quaternion operator-() const;
    inline Quaternion operator~() const;

    inline Bool4 operator==(const Quaternion& rh) const;
    inline Bool4 operator!=(const Quaternion& rh) const;
    
    inline Float3 operator*(const Float3& rh) const;
};


R_PUBLIC_API Quaternion normalize(const Quaternion& quat);
R_PUBLIC_API F32        norm(const Quaternion& quat);
R_PUBLIC_API F32        norm2(const Quaternion& quat);
R_PUBLIC_API F32        dot(const Quaternion& a, const Quaternion& b);
R_PUBLIC_API Quaternion lookRotation(const Float3& dir, const Float3& up);
R_PUBLIC_API Quaternion conjugate(const Quaternion& quat);
R_PUBLIC_API Quaternion inverse(const Quaternion& quat);
R_PUBLIC_API Quaternion angleAxis(const Float3& axis, F32 radians);
R_PUBLIC_API Quaternion eulerToQuat(const Float3& euler);
R_PUBLIC_API Float3     quatToEuler(const Quaternion& quat);
R_PUBLIC_API Matrix44   quatToMat44(const Quaternion& quat);
R_PUBLIC_API Quaternion slerp(const Quaternion& a, const Quaternion& b, F32 t);
} // Math
} // Recluse