//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Math/Vector2.hpp"

namespace Recluse {
namespace Math {

struct R_PUBLIC_API Matrix22 
{
    struct { F32 m[4]; };

    F32& operator[](U32 i) { return m[i]; }
    F32 operator[](U32 i) const { return m[i]; }

    Matrix22
        (
            F32 a00 = 1.0f, F32 a01 = 0.0f,
            F32 a10 = 0.0f, F32 a11 = 1.0f
        );

    Matrix22
        (
            const Float2& row0,
            const Float2& row1
        );

    inline Matrix22 operator+(const Matrix22& rh) const;
    inline Matrix22 operator-(const Matrix22& rh) const;
    inline Matrix22 operator*(const Matrix22& rh) const;
    inline Matrix22 operator+(F32 scalar) const;
    inline Matrix22 operator-(F32 scalar) const;
    inline Matrix22 operator*(F32 scalar) const;
    inline Matrix22 operator/(F32 scalar) const;
    inline Matrix22 operator-() const;

    F32             get(U32 row, U32 col) const;
    F32&            get(U32 row, U32 col);

    F32             operator()(U32 row, U32 col) const { return get(row, col); }
    F32&            operator()(U32 row, U32 col) { return get(row, col); }
};


R_PUBLIC_API Matrix22 translate(const Matrix22& base, const Float2& t);
R_PUBLIC_API Matrix22 transpose(const Matrix22& m);
R_PUBLIC_API F32      determinant(const Matrix22& m);
R_PUBLIC_API Matrix22 inverse(const Matrix22& m);
} // Math
} // Recluse