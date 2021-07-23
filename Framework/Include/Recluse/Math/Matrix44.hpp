//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {


struct R_EXPORT Matrix44 {
    F32 d[16];

    Matrix44(F32 a00 = 1.0f, F32 a01 = 0.0f, F32 a02 = 0.0f, F32 a03 = 0.0f,
             F32 a10 = 0.0f, F32 a11 = 1.0f, F32 a12 = 0.0f, F32 a13 = 0.0f,
             F32 a20 = 0.0f, F32 a21 = 0.0f, F32 a22 = 1.0f, F32 a23 = 0.0f,
             F32 a30 = 0.0f, F32 a31 = 0.0f, F32 a32 = 0.0f, F32 a33 = 1.0f);
    Matrix44(const Float4& row0,
             const Float4& row1,
             const Float4& row2,
             const Float4& row3);

    F32 get(U32 row, U32 col) const;
    F32& get(U32 row, U32 col);
    F32& operator[](U32 ix) { return d[ix]; }
    F32 operator[](U32 ix) const { return d[ix]; }

    inline Matrix44 operator+(const Matrix44& rh) const;
    inline Matrix44 operator-(const Matrix44& rh) const;
    inline Matrix44 operator*(const Matrix44& rh) const;
    
    inline Matrix44 operator+(F32 scalar) const;
    inline Matrix44 operator-(F32 scalar) const;
    inline Matrix44 operator*(F32 scalar) const;
    inline Matrix44 operator/(F32 scalar) const;  
};


Matrix44 R_EXPORT rotate(const Matrix44& init, const Float3& axis, F32 radius);
Matrix44 R_EXPORT transpose(const Matrix44& init);
Matrix44 R_EXPORT identity();
Matrix44 R_EXPORT translate(const Matrix44& init, const Float3& trans);
Matrix44 R_EXPORT scale(const Matrix44& init, const Float4& scalar);
Matrix44 R_EXPORT adjugate(const Matrix44& init);
Matrix44 R_EXPORT inverse(const Matrix44& init);
F32      R_EXPORT determinant(const Matrix44& init);
Matrix44 R_EXPORT perspectiveLH(F32 fov, F32 aspect, F32 ne, F32 fa);
Matrix44 R_EXPORT perspectiveRH(F32 fov, F32 aspect, F32 ne, F32 fa);
Matrix44 R_EXPORT orthographicLH(F32 top, F32 bottom, F32 left, F32 right, F32 ne, F32 fa);
Matrix44 R_EXPORT orthographicRH(F32 top, F32 bottom, F32 left, F32 right, F32 ne, F32 fa);
Matrix44 R_EXPORT lookAt(const Float3& position, const Float3& target, const Float3& up);
Matrix44 R_EXPORT lookAtRH(const Float3& position, const Float3& target, const Float3& up);
Matrix44 R_EXPORT operator*(const Matrix44& lh, const Float4& rh);
} // Recluse