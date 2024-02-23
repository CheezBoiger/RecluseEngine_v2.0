// 
#pragma once

#include "Recluse/Math/MathIntrinsics.hpp"
#include "Recluse/Math/Vector3.hpp"

namespace Recluse {
namespace Math {
struct Matrix44;
struct Matrix43;


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
    inline Float4(const Float2& xy, F32 z = 0.f, F32 w = 0.f)
        : x(xy.x), y(xy.y), z(z), w(w) { }

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


struct R_PUBLIC_API Int4
{
    union
    {
        struct { I32 x, y, z, w; };
        struct { I32 s, t, r, q; };
        struct { I32 r, g, b, a; };
    };

    Int4(I32 x = 0, I32 y = 0, I32 z = 0, I32 w = 0)
        : x(x), y(y), z(z), w(w) { }

    inline I32& operator[](U32 i) { return (&x)[i]; }
    inline I32 operator[](U32 i) const { return (&x)[i]; }

    inline Int4 operator+(const Int4& rh) const
    {
        return Int4(x + rh.x, y + rh.y, z + rh.z, w + rh.w);
    }

    inline Int4 operator-(const Int4& rh) const
    {
        return Int4(x - rh.x, y - rh.y, z - rh.z, w - rh.w);
    }

    inline Int4 operator*(const Int4& rh) const
    {
        return Int4(x * rh.x, y * rh.y, z * rh.z, w * rh.w);
    }
};


struct R_PUBLIC_API UByte4
{
    union
    {
        struct { U8 x, y, z, w; };
        struct { U8 s, t, r, q; };
        struct { U8 r, g, b, a; };
    };

    UByte4(U8 x = 0, U8 y = 0, U8 z = 0, U8 w = 0)
        : x(x), y(y), z(z), w(w) { }
    
    UByte4 operator-(const UByte4& rh) const
    {
        return UByte4(x - rh.x, y - rh.y, z - rh.z, w - rh.w);
    }

    UByte4 operator+(const UByte4& rh) const
    {
        return UByte4(x + rh.x, y - rh.y, z - rh.z, w - rh.w);
    }

    UByte4 operator*(const UByte4& rh) const
    {
        return UByte4(x * rh.x, y * rh.y, z * rh.z, w * rh.w);
    }

    inline U8& operator[](U32 idx) { return (&x)[idx]; }
    inline U8 operator[](U32 idx) const { return (&x)[idx]; }
};


struct R_PUBLIC_API Byte4
{
    union
    {
        struct { I8 x, y, z, w; };
        struct { I8 s, t, r, q; };
        struct { I8 r, g, b, a; };
    };

    Byte4(I8 x = 0, I8 y = 0, I8 z = 0, I8 w = 0)
        : x(x), y(y), z(z), w(w) { }
    
    Byte4 operator-(const UByte4& rh) const
    {
        return Byte4(x - rh.x, y - rh.y, z - rh.z, w - rh.w);
    }

    Byte4 operator+(const Byte4& rh) const
    {
        return Byte4(x + rh.x, y - rh.y, z - rh.z, w - rh.w);
    }

    inline I8& operator[](U32 idx) { return (&x)[idx]; }
    inline I8 operator[](U32 idx) const { return (&x)[idx]; }
};


typedef UByte4 Color4;

// Converts color vector to unit float4.
R_PUBLIC_API Float4 colorToFloat(const Color4& color);

// Converts unit float4 to color vector.
R_PUBLIC_API Color4 floatToColor(const Float4& color);

// Dot product of a . b
R_PUBLIC_API F32 dot(const Float4& a, const Float4& b);

// Dot product of a . b
R_PUBLIC_API U32 dot(const UByte4& a, const UByte4& b);

// Dot product of a . b
R_PUBLIC_API U32 dot(const UInt4& a, const UInt4& b);

// Dot product of a . b
R_PUBLIC_API U32 dot(const Int4& a, const Int4& b);

// The Euclidean length of float4 vector.
R_PUBLIC_API F32 length(const Float4& a);

// The Eucidean length of float4 vector, but not squared to obtain the actual length of the magnitude.
R_PUBLIC_API F32 length2(const Float4& a);

// Euclidean distance between point p0, and p1.
R_PUBLIC_API F32 dist(const Float4& p0, const Float4& p1);

// Euclidean distance between point p0, and p1.
R_PUBLIC_API I32 dist(const UInt4& p0, const UInt4& p1);

// Euclidean distance between point p0, and p1.
R_PUBLIC_API I32 dist(const UByte4& p0, const UByte4& p1);

// [1 x 4] * [4 x 4] = [1 x 4]
R_PUBLIC_API Float4 operator*(const Float4& lh, const Matrix44& rh);
// [1 x 4] * [4 x 3] = [1 x 4]
R_PUBLIC_API Float4 operator*(const Float4& lh, const Matrix43& rh);

// Normalizes the float4 variable, which become a unit vector [0, 1].
R_PUBLIC_API Float4 normalize(const Float4& lh);

// See if any component in the vector is a nonzero number.
R_PUBLIC_API Bool any(const Float4& a);
// Check if all components in the vector are true.
R_PUBLIC_API Bool all(const Float4& a);
} // Math
} // Recluse