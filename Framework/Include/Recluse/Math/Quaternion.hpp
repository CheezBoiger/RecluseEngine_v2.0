//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {


struct Quaternion {

    F32 x, y, z, w;
    
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
    
    inline Float3 operator*(const Float3& rh) const;
};


Quaternion  normalize(const Quaternion& quat);
F32         norm(const Quaternion& quat);
F32         norm2(const Quaternion& quat);
F32         dot(const Quaternion& a, const Quaternion& b);
Quaternion  lookRotation(const Float3& dir, const Float3& up);
Quaternion  conjugate(const Quaternion& quat);
Quaternion  inverse(const Quaternion& quat);
Quaternion  angleAxis(const Float3& axis, F32 radians);
Quaternion  eulerToQuat(const Float3& euler);
Float3      quatToEuler(const Quaternion& quat);
Matrix44    quatToMat44(const Quaternion& quat);
Quaternion  slerp(const Quaternion& a, const Quaternion& b, F32 t);
} // Recluse