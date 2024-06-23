//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/MathIntrinsics.hpp"

namespace Recluse {
namespace Math {
struct Matrix44;

struct R_PUBLIC_API Matrix43
{
public:
    union 
    {
        F32 m[12];
        struct 
        {
            __m128 row0;
            __m128 row1;
            __m128 row2;
        };
    };


    Matrix43(F32 a00 = 0.0f, F32 a01 = 0.0f, F32 a02 = 0.0f,
             F32 a10 = 0.0f, F32 a11 = 0.0f, F32 a12 = 0.0f,
             F32 a20 = 0.0f, F32 a21 = 0.0f, F32 a22 = 0.0f,
             F32 a30 = 0.0f, F32 a31 = 0.0f, F32 a32 = 0.0f);

    // Convert to 4x4 square matrix.
    operator Matrix44 ();
    operator Matrix44 () const;

    F32 get(U32 row, U32 col) const;
    F32& get(U32 row, U32 col);

    F32 operator()(U32 row, U32 col) const { return get(row, col); }
    F32& operator()(U32 row, U32 col) { return get(row, col); }
};


typedef Matrix43 Mat43;
typedef Matrix43 M43;
} // Math
} // Recluse