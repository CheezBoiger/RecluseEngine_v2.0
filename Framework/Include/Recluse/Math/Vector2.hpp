//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace Math {

struct R_PUBLIC_API Float2 
{ 
    union 
    {
        struct { F32 x, y; };
        struct { F32 r, g; };
        struct { F32 u, v; };
    };

    inline Float2(F32 x = 0.f, F32 y = 0.f)
        : x(x), y(y) { }

    inline F32& operator[](I32 i) { return (&x)[i]; }
    inline const F32& operator[](I32 i) const { return (&x)[i]; }
    inline Float2 operator+(const Float2& right) const;
    inline Float2 operator-(const Float2& right) const;
    inline Float2 operator-() const;
    inline Float2 operator*(const Float2& right) const;
    inline Float2 operator/(const Float2& right) const;
    inline Float2 operator*(F32 scalar) const;
    inline Float2 operator/(F32 scalar) const;
    inline Float2 operator+(F32 scalar) const;
    inline Float2 operator-(F32 scalar) const;
    inline Float2 operator==(const Float2& right) const;
    inline Float2 operator&&(const Float2& rh) const;
    inline Float2 operator||(const Float2& rh) const;
    inline Float2 operator<(const Float2& rh) const;
    inline Float2 operator>(const Float2& rh) const;
    inline Float2 operator>=(const Float2& rh) const;
    inline Float2 operator<=(const Float2& rh) const;

    inline Float2 operator==(F32 scalar) const;
    inline Float2 operator<(F32 scalar) const;
    inline Float2 operator>(F32 scalar) const;
    inline Float2 operator<=(F32 scalar) const;
    inline Float2 operator>=(F32 scalar) const;

    friend Float2 operator*(F32 scalar, const Float2& rh);
    friend Float2 operator+(F32 scalar, const Float2& rh);
    friend Float2 operator-(F32 scalar, const Float2& rh);
    friend Float2 operator/(F32 scalar, const Float2& rh);
    
};


struct R_PUBLIC_API Int2 
{
    union 
    {
        struct { I32 x, y; };
        struct { I32 r, g; };
        struct { I32 u, v; };
    };

    inline Int2(I32 x = 0.f, I32 y = 0.f)
        : x(x), y(y) { }

    I32& operator[](U32 idx) { return (&x)[idx]; }
    const I32& operator[](U32 idx) const { return (&x)[idx]; }

    inline Int2 operator+(const Int2& rh) const;
    inline Int2 operator-(const Int2& rh) const;
    inline Int2 operator-() const;
    inline Int2 operator*(const Int2& rh) const;
    inline Int2 operator/(const Int2& rh) const;
    inline Int2 operator*(I32 scalar) const;
    inline Int2 operator/(I32 scalar) const;
    inline Int2 operator+(I32 scalar) const;
    inline Int2 operator-(I32 scalar) const;
    inline Int2 operator&(const Int2& rh) const;
    inline Int2 operator|(const Int2& rh) const;
    inline Int2 operator^(const Int2& rh) const;
    inline Int2 operator==(const Int2& rh) const;
    inline Int2 operator&&(const Int2& rh) const;
    inline Int2 operator||(const Int2& rh) const;
    inline Int2 operator<(const Int2& rh) const;
    inline Int2 operator>(const Int2& rh) const;
    inline Int2 operator>=(const Int2& rh) const;
    inline Int2 operator<=(const Int2& rh) const;
    inline Int2 operator<<(U32 shft) const;
    inline Int2 operator>>(U32 shft) const;
};

class R_PUBLIC_API UInt2 
{
    union 
    {
        struct { U32 x, y; };
        struct { U32 r, g; };
        struct { U32 u, v; };
    };

    inline UInt2(U32 x = 0.f, U32 y = 0.f)
        : x(x), y(y) { }

    inline UInt2 operator+(const UInt2& rh) const;
    inline UInt2 operator-(const UInt2& rh) const;
    inline UInt2 operator*(const UInt2& rh) const;
    inline UInt2 operator/(const UInt2& rh) const;
    
    inline UInt2 operator+(U32 scalar) const;
    inline UInt2 operator-(U32 scalar) const;
    inline UInt2 operator*(U32 scalar) const;
    inline UInt2 operator/(U32 scalar) const;
};


struct R_PUBLIC_API Short2 
{
    union 
    {
        struct { I16 x, y; };
        struct { I16 r, g; };
        struct { I16 u, v; };
    };
};


struct R_PUBLIC_API UShort2 
{
    union 
    {
        struct { U16 x, y; };
        struct { U16 r, g; };
        struct { U16 u, v; };
    };
};

R_PUBLIC_API F32        dot(const Float2& a, const Float2& b);
R_PUBLIC_API F32        dot(const Int2& a, const Int2& b);
R_PUBLIC_API F32        dot(const UInt2& a, const UInt2& b);

R_PUBLIC_API F32        length(const Float2& a);
R_PUBLIC_API F32        length2(const Float2& a);

R_PUBLIC_API Float2     normalize(const Float2& a);
R_PUBLIC_API Int2       normalize(const Int2& a);
R_PUBLIC_API UInt2      normalize(const UInt2& a);

R_PUBLIC_API Float2     reflect(const Float2& incidence, const Float2& normal);

// Check if any component of the vector is nonzero.
R_PUBLIC_API Bool       any(const Float2& a);
// Check if all components of the vector are nonzero.
R_PUBLIC_API Bool       all(const Float2& a);
} // Math
} // Recluse