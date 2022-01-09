//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector3.hpp"

namespace Recluse {


struct R_PUBLIC_API Matrix33 
{ 
    F32 m[9];

    Matrix33
        (
            F32 a00 = 1.0f, F32 a01 = 0.0f, F32 a02 = 0.0f,
            F32 a10 = 0.0f, F32 a11 = 1.0f, F32 a12 = 0.0f,
            F32 a20 = 0.0f, F32 a21 = 0.0f, F32 a22 = 1.0f
        );

    Matrix33
        (
            const Float3& row0,
            const Float3& row1,
            const Float3& row2
        );

    F32& operator[](U32 i) { return m[i]; }
    F32 operator[](U32 i) const { return m[i]; }

    inline Matrix33 operator+(const Matrix33& rh) const;
    inline Matrix33 operator-(const Matrix33& rh) const;
    inline Matrix33 operator*(const Matrix33& rh) const;
    
    inline Matrix33 operator+(F32 scalar) const;
    inline Matrix33 operator-(F32 scalar) const;
    inline Matrix33 operator*(F32 scalar) const;
};
} // Recluse