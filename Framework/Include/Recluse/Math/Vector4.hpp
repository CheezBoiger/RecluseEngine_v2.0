// 
#pragma once

#include "Recluse/Math/MathIntrinsics.hpp"
#include "Recluse/Math/Vector3.hpp"

namespace Recluse {
namespace Math {
struct Matrix44;


struct Bool4
{
    union
    {
        struct { Bool x, y, z, w; };
    };

    Bool4(Bool x = false, Bool y = false, Bool z = false, Bool w = false)
        : x(x), y(y), z(z), w(w) { }

    // Check if any values are true.
    Bool any() const { return (x || y || z || w); }

    // Check if all values are true.
    Bool all() const { return (x && y && z && w); }

    Bool4 operator !() const { return Bool4(!x, !y, !z, !w); } 
};

// Float4 vector struct.
struct R_PUBLIC_API Float4 
{
    union 
    {
        struct { F32 x, y, z, w; };
        struct { F32 r, g, b, a; };
        struct { F32 s, t, r, q; };
        struct { F32 u, v, w, c; };
        __m128 row;
    };

    inline Float4(F32 x = 0.f, F32 y = 0.f, F32 z = 0.f, F32 w = 0.f)
        : x(x), y(y), z(z), w(w) { }
    inline Float4(const Float3& xyz, F32 w = 0.f)
        : x(xyz.x), y(xyz.y), z(xyz.z), w(w) { }
    inline Float4(const Float2& xy, const Float2& zw)
        : x(xy.x), y(xy.y), z(zw.x), w(zw.y) { }

    F32& operator[](U32 i) { return (&x)[i]; }
    F32 operator[](U32 i) const { return (&x)[i]; }

    inline Float4           operator+(const Float4& rh) const;
    inline Float4           operator-(const Float4& rh) const;
    inline Float4           operator*(const Float4& rh) const;
    inline Float4           operator/(const Float4& rh) const;
    inline Float4           operator+(F32 scalar) const;
    inline Float4           operator-(F32 scalar) const;
    inline Float4           operator*(F32 scalar) const;
    inline Float4           operator/(F32 scalar) const;
    inline Float4           operator-() const;

    inline Bool4             operator==(const Float4& rh) const;
    inline Bool4             operator!=(const Float4& rh) const;
    inline Bool4             operator&&(const Float4& rh) const;
    inline Bool4             operator||(const Float4& rh) const;
    inline Bool4             operator<(const Float4& rh) const;
    inline Bool4             operator>(const Float4& rh) const;
    inline Bool4             operator>=(const Float4& rh) const;
    inline Bool4             operator<=(const Float4& rh) const;
    inline Bool4             operator==(F32 scalar) const;
    inline Bool4             operator<(F32 scalar) const;
    inline Bool4             operator>(F32 scalar) const;
    inline Bool4             operator<=(F32 scalar) const;
    inline Bool4             operator>=(F32 scalar) const;

    inline friend Float4    operator+(F32 scalar, const Float4& rh);
    inline friend Float4    operator-(F32 scalar, const Float4& rh);
    inline friend Float4    operator*(F32 scalar, const Float4& rh);
    inline friend Float4    operator/(F32 scalar, const Float4& rh);
    
};


struct R_PUBLIC_API UInt4 
{
    union 
    {
        struct { U32 x, y, z, w; };
        struct { U32 s, t, r, q; };
        struct { U32 r, g, b, a; };
    };

    UInt4(U32 x = 0, U32 y = 0, U32 z = 0, U32 w = 0)
        : x(x), y(y), z(z), w(w) { }

    inline U32& operator[](U32 i) { return (&x)[i]; }
    inline U32 operator[](U32 i) const { return (&x)[i]; }

    inline UInt4 operator+(const UInt4& rh) const;
    inline UInt4 operator-(const UInt4& rh) const;
    inline UInt4 operator*(const UInt4& rh) const;
    inline UInt4 operator/(const UInt4& rh) const;
    
    inline UInt4 operator-() const;

    inline UInt4 operator+(U32 scalar) const;
    inline UInt4 operator-(U32 scalar) const;
    inline UInt4 operator*(U32 scalar) const;
    inline UInt4 operator/(U32 scalar) const;

    inline UInt4 operator&(const UInt4& rh) const;
    inline UInt4 operator|(const UInt4& rh) const;
    inline UInt4 operator^(const UInt4& rh) const;
    inline UInt4 operator~() const;

    inline UInt4 operator&&(const UInt4& rh) const;
    inline UInt4 operator||(const UInt4& rh) const;
    inline UInt4 operator==(const UInt4& rh) const;
    inline UInt4 operator<=(const UInt4& rh) const;
    inline UInt4 operator>=(const UInt4& rh) const;
    inline UInt4 operator>(const UInt4& rh) const;
    inline UInt4 operator<(const UInt4& rh) const;
};

R_PUBLIC_API F32 dot(const Float4& a, const Float4& b);
R_PUBLIC_API F32 length(const Float4& a);
R_PUBLIC_API F32 length2(const Float4& a);
// [1 x 4] * [4 x 4} = [1 x 4]
R_PUBLIC_API Float4 operator*(const Float4& lh, const Matrix44& rh);
R_PUBLIC_API Float4 normalize(const Float4& lh);

// See if any component in the vector is a nonzero number.
R_PUBLIC_API Bool any(const Float4& a);
} // Math
} // Recluse