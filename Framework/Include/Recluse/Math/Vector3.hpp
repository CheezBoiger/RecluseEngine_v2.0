//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector2.hpp"


namespace Recluse {
namespace Math {

struct Float4;

struct Bool3
{
    union
    {
        struct { Bool x, y, z; };
    };

    Bool3(Bool x = false, Bool y = false, Bool z = false)
        : x(x), y(y), z(z) { }

    // Check if any boolean values are true.
    Bool any() const { return x || y || z; };

    // Check if all boolean values are true.
    Bool all() const { return x && y && z; };

    Bool3 operator!() const { return Bool3(!x, !y, !z); }
};

// Float Vector data structure.
struct R_PUBLIC_API Float3 
{
    union 
    {
        struct { F32 x, y, z; };
        struct { F32 r, g, b; };
        struct { F32 u, v, w; };
    };

    Float3(F32 x = 0.f, F32 y = 0.f, F32 z = 0.f)
        : x(x), y(y), z(z) { }
    Float3(const Float2& xy, F32 z = 0.f)
        : x(xy.x), y(xy.y), z(z) { }

    inline F32& operator[](U32 idx) { return (&x)[idx]; }
    inline F32 operator[](U32 idx) const { return (&x)[idx]; }

    inline Float3 operator+(const Float3& rh) const;
    inline Float3 operator-(const Float3& rh) const;
    inline Float3 operator-() const;
    inline Float3 operator*(const Float3& rh) const;
    inline Float3 operator/(const Float3& rh) const;
    inline Float3 operator*(F32 scalar) const;
    inline Float3 operator/(F32 scalar) const;
    inline Float3 operator+(F32 scalar) const;
    inline Float3 operator-(F32 scalar) const;
    inline Bool3  operator==(const Float3& rh) const;
    inline Bool3  operator!=(const Float3& rh) const;
    inline Bool3  operator&&(const Float3& rh) const;
    inline Bool3  operator||(const Float3& rh) const;
    inline Bool3  operator<(const Float3& rh) const;
    inline Bool3  operator>(const Float3& rh) const;
    inline Bool3  operator>=(const Float3& rh) const;
    inline Bool3  operator<=(const Float3& rh) const;

    inline Bool3  operator==(F32 scalar) const;
    inline Bool3  operator<(F32 scalar) const;
    inline Bool3  operator>(F32 scalar) const;
    inline Bool3  operator<=(F32 scalar) const;
    inline Bool3  operator>=(F32 scalar) const;

    inline R_PUBLIC_API friend Float3 operator*(F32 scalar, const Float3& rh);
    inline R_PUBLIC_API friend Float3 operator+(F32 scalar, const Float3& rh);
    inline R_PUBLIC_API friend Float3 operator-(F32 scalar, const Float3& rh);
    inline R_PUBLIC_API friend Float3 operator/(F32 scalar, const Float3& rh);

};


struct Int3 
{
    union 
    {
        struct { I32 x, y, z; };
        struct { I32 r, g, b; };
        struct { I32 u, v, w; };
    };

    Int3(I32 x = 0, I32 y = 0, I32 z = 0)
        : x(x), y(y), z(z) { }
    Int3(const Int2& a, I32 z = 0)
        : x(a.x), y(a.y), z(z) { }

    inline Int3 operator+(const Int3& rh) const
    {
        return Int3(x + rh.x, y + rh.y, z + rh.z);
    }

    inline Int3 operator-(const Int3& rh) const
    {
        return Int3(x - rh.x, y - rh.y, z - rh.z);
    }

    inline Int3 operator-() const
    {
        return Int3(-x, -y, -z);
    }

    inline Int3 operator+(I32 scalar) const
    {
        return Int3(x + scalar, y + scalar, z + scalar);
    }

    inline Int3 operator-(I32 scalar) const
    {
        return Int3(x - scalar, y - scalar, z - scalar);
    }

    inline Int3 operator/(I32 scalar) const
    {
        return Int3(x / scalar, y / scalar, z / scalar);
    }

    inline Int3 operator*(I32 scalar) const
    {
        return Int3(x * scalar, y * scalar, z * scalar);
    }

    inline Int3 operator*(const Int3& rh) const
    {
        return Int3(x * rh.x, y * rh.y, z * rh.z);
    }

    inline Int3 operator&(const Int3& rh) const;
    inline Int3 operator|(const Int3& rh) const;

    inline friend Int3 operator+(I32 scalar, const Int3& rh);
    inline friend Int3 operator-(I32 scalar, const Int3& rh);
    inline friend Int3 operator*(I32 scalar, const Int3& rh);
    inline friend Int3 operator/(I32 scalar, const Int3& rh);

    inline I32& operator[](U32 i) { return (&x)[i]; }
    inline I32 operator[](U32 i) const { return (&x)[i]; }
};


struct R_PUBLIC_API UInt3 
{
    union 
    {
        struct { U32 x, y, z; };
        struct { U32 r, g, b; };
        struct { U32 u, v, w; };
    };
};


struct R_PUBLIC_API Short3 
{
    union 
    {
        struct { I16 x, y, z; };
        struct { I16 r, g, b; };
        struct { I16 u, v, w; };
    };
};


struct R_PUBLIC_API UShort3 
{
    union 
    {
        struct { U16 x, y, z; };
        struct { U16 r, g, b; };
        struct { U16 u, v, w; };
    };
};


struct R_PUBLIC_API UByte3
{
    union
    {
        struct { U8 x, y, z; };
        struct { U8 r, g, b; };
        struct { U8 u, v, w; };
    };

    UByte3(U8 x = 0, U8 y = 0, U8 z = 0)
        : x(x), y(y), z(z)
    { }

    inline U8& operator[](U32 idx) { return (&x)[idx]; }
    inline U8 operator[](U32 idx) const { return (&x)[idx]; }
};


typedef UByte3 Color3;

R_PUBLIC_API Float3     cross(const Float3& lh, const Float3& rh);
R_PUBLIC_API F32        dot(const Float3& lh, const Float3& rh);
R_PUBLIC_API U32        dot(const Int3& lh, const Int3& rh);
R_PUBLIC_API F32        length(const Float3& v);
R_PUBLIC_API F32        length2(const Float3& v);
R_PUBLIC_API Float3     normalize(const Float3& v);

// Euclidean distance between point p0, and p1.
R_PUBLIC_API F32        dist(const Float3& p0, const Float3& p1);
R_PUBLIC_API U32        dist(const UByte3& p0, const UByte3& p1);
R_PUBLIC_API U32        dist(const UInt3& p0, const UInt3& p1);

R_PUBLIC_API Color3     floatToColor(const Float3& color);
R_PUBLIC_API Float3     colorToFloat(const Color3& color);

// Obtain the reflection vector from the incidence, which is governed by the facing normal.
R_PUBLIC_API Float3     reflect(const Float3& incidence, const Float3& normal);

// Obtain the refraction vector, which is governed by the facing normal, and ratio of indices of refraction.
R_PUBLIC_API Float3     refract(const Float3& incidence, const Float3& normal, F32 eta);
// check if any component in the vector is nonzero.
R_PUBLIC_API Bool       any(const Float3& a);
// check if all components in the vector are nonzero.
R_PUBLIC_API Bool       all(const Float3& a);


typedef Float3  FVector3;
typedef Int3    IVector3;
typedef UInt3   UVector3;
typedef Short3  ShVector3;
} // Math
} // Recluse