//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {

struct Float2 { 
    union {
        struct { F32 x, y; };
        struct { F32 r, g; };
        struct { F32 u, v; };
    };

    Float2(F32 x = 0.f, F32 y = 0.f)
        : x(x), y(y) { }

    F32& operator[](I32 i) { return (&x)[i]; }
    const F32& operator[](I32 i) const { return (&x)[i]; }
  
    Float2 operator+(const Float2& right) const {
        return Float2(x + right.x, y + right.y);
    }  

    Float2 operator-(const Float2& right) const {
        return Float2(x - right.x, y - right.y);
    }

    Float2 operator-() const {
        return Float2(-x, -y);
    }

    Float2 operator*(const Float2& right) const {
        return Float2(x * right.x, y * right.y);
    }

    Float2 operator/(const Float2& right) const {
        return Float2(x / right.x, y / right.y);
    }

    Float2 operator*(F32 scalar) const {
        return Float2(x * scalar, y * scalar);
    }

    Float2 operator/(F32 scalar) const {
        return Float2(x / scalar, y / scalar);
    }

    Float2 operator+(F32 scalar) const {
        return Float2(x + scalar, y + scalar);
    }

    Float2 operator-(F32 scalar) const {
        return Float2(x - scalar, y - scalar);
    }

    Float2 operator==(const Float2& right) const {
        return Float2(F32(x == right.x), F32(y == right.y));
    }
};


struct Int2 {
    union {
        struct { I32 x, y; };
        struct { I32 r, g; };
        struct { I32 u, v; };
    };

    Int2(I32 x = 0.f, I32 y = 0.f)
        : x(x), y(y) { }

    Int2 operator+(const Int2& right) const {
        return Int2(x + right.x, y + right.y);
    }

    Int2 operator-(const Int2& right) const {
        return Int2(x - right.x, y - right.y);
    }

    Int2 operator-() const {
        return Int2(-x, -y);
    }

    Int2 operator*(const Int2& right) const {
        return Int2(x * right.x, y * right.y);
    }

    Int2 operator/(const Int2& right) const {
        return Int2(x / right.x, y / right.y);
    }

    Int2 operator*(I32 scalar) const {
        return Int2(x * scalar, y * scalar);
    }

    Int2 operator/(I32 scalar) const {
        return Int2(x / scalar, y / scalar);
    }

    Int2 operator+(I32 scalar) const {
        return Int2(x + scalar, y + scalar);
    }

    Int2 operator-(I32 scalar) const {
        return Int2(x - scalar, y - scalar);
    }

    Int2 operator&(const Int2& right) const {
        return Int2(x & right.x, y & right.y);
    }

    Int2 operator|(const Int2& right) const {
        return Int2(x | right.x, y | right.y);
    }
};

class UInt2 {
    union {
        struct { U32 x, y; };
        struct { U32 r, g; };
        struct { U32 u, v; };
    };
};


struct Short2 {
    union {
        struct { I16 x, y; };
        struct { I16 r, g; };
        struct { I16 u, v; };
    };
};


struct UShort2 {
    union {
        struct { U16 x, y; };
        struct { U16 r, g; };
        struct { U16 u, v; };
    };
};


F32 dot(const Float2& a, const Float2& b);
F32 dot(const Int2& a, const Int2& b);
F32 dot(const UInt2& a, const UInt2& b);

F32 sqrt(const Float2& a);
F32 length(const Float2& a);
F32 length2(const Float2& a);

Float2 normalize(const Float2& a);
Int2 normalize(const Int2& a);
UInt2 normalize(const UInt2& a);
} // Recluse