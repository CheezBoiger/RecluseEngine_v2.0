//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/MathIntrinsics.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {
namespace Math {

// Quaternions are mathematical members that represent systems with real numbers.
// They are defined as having the form a + bi + cj + dk where a,b,c,d represent real numbers, 
// and 1, i, j, k represent the basis vectors or elements. 
//
// These are representations, in which work perfectly for rotations, as the i, j, k, can represent an
// axis of rotation in 3 dimensions, and the final component can represent the degree of rotation. 
// This in turn can be universally understood in 4 effective components of the quaternion, rather than 
// euler represention (to which euler angles require knowledge of both the 3 axis of rotations, their order,
// and possible loss of degree of freedom due to gimbal-locking.)
// Quaternions are useful in solving these issues.
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

// Quaternion normalization.
R_PUBLIC_API Quaternion normalize(const Quaternion& quat);
R_PUBLIC_API F32        norm(const Quaternion& quat);
R_PUBLIC_API F32        norm2(const Quaternion& quat);
R_PUBLIC_API F32        dot(const Quaternion& a, const Quaternion& b);
R_PUBLIC_API Quaternion lookRotation(const Float3& dir, const Float3& up);
R_PUBLIC_API Quaternion conjugate(const Quaternion& quat);
R_PUBLIC_API Quaternion inverse(const Quaternion& quat);

// Angle-Axis representation computation for a float axis, with rotation radians.
R_PUBLIC_API Quaternion angleAxis(const Float3& axis, F32 radians);

// Converts an euler represented vector to a quaternion.
R_PUBLIC_API Quaternion eulerToQuat(const Float3& euler);

// Converts a quaternion to an euler represented vector.
R_PUBLIC_API Float3     quatToEuler(const Quaternion& quat);

// Converts a quaternion to a 4x4 matrix.
R_PUBLIC_API Matrix44   quatToMat44(const Quaternion& quat);

// Spherical linear interpolation with two quaternions of time t.
R_PUBLIC_API Quaternion slerp(const Quaternion& a, const Quaternion& b, F32 t);
} // Math
} // Recluse