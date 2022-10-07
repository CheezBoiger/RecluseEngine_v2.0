//
#include "Recluse/Math/Vector2.hpp"

namespace Recluse {
namespace Math {

Float2 Float2::operator+(const Float2& rh) const
{
    return Float2(x + rh.x, y + rh.y);
}


Float2 Float2::operator-(const Float2& rh) const
{
    return Float2(x - rh.x, y - rh.y);
}


Float2 Float2::operator-() const
{
    return Float2(-x, -y);
}


Float2 Float2::operator*(const Float2& rh) const
{
    return Float2(x * rh.x, y * rh.y);
}


Float2 Float2::operator/(const Float2& rh) const
{
    return Float2(x / rh.x, y / rh.y);
}


Float2 Float2::operator*(F32 scalar) const
{
    return Float2(x * scalar, y * scalar);
}


Float2 Float2::operator/(F32 scalar) const 
{
    return Float2(x / scalar, y / scalar);
}


Float2 Float2::operator+(F32 scalar) const
{
    return Float2(x + scalar, y + scalar);
}


Float2 Float2::operator-(F32 scalar) const
{
    return Float2(x - scalar, y - scalar);
}


Float2 Float2::operator==(const Float2& rh) const
{
    return Float2(x == rh.x, y == rh.y);
}


Float2 Float2::operator&&(const Float2& rh) const
{
    return Float2(x && rh.x, y && rh.y);
}


Float2 Float2::operator||(const Float2& rh) const
{
    return Float2(x || rh.x, y || rh.y);
}


Float2 Float2::operator<(const Float2& rh) const
{
    return Float2(x < rh.x, y < rh.y);
}


Float2 Float2::operator>(const Float2& rh) const
{
    return Float2(x > rh.x, y > rh.y);
}


Float2 Float2::operator<=(const Float2& rh) const
{
    return Float2(x <= rh.x, y <= rh.y);
}


Float2 Float2::operator>=(const Float2& rh) const
{
    return Float2(x >= rh.x, y >= rh.y);
}


Int2 Int2::operator-() const
{
    return Int2(-x, -y);
}


Int2 Int2::operator*(const Int2& rh) const
{
    return Int2(x * rh.x, y * rh.y);
}


Int2 Int2::operator/(const Int2& rh) const
{
    return Int2(x / rh.x, y / rh.y);
}


Int2 Int2::operator*(I32 scalar) const
{
    return Int2(x * scalar, y * scalar);
}


Int2 Int2::operator/(I32 scalar) const
{
    return Int2(x / scalar, y / scalar);
}


Int2 Int2::operator-(I32 scalar) const
{
    return Int2(x - scalar, y - scalar);
}


Int2 Int2::operator&(const Int2& rh) const
{
    return Int2(x & rh.x, y & rh.y);
}


Int2 Int2::operator|(const Int2& rh) const
{
    return Int2(x | rh.x, y | rh.y);
}


Int2 Int2::operator^(const Int2& rh) const
{
    return Int2(x ^ rh.x, y ^ rh.y);
}


Int2 Int2::operator+(I32 scalar) const
{
    return Int2(x + scalar, y + scalar);
}

Int2 Int2::operator<<(U32 shft) const
{
    return Int2(x << shft, y << shft);
}


Int2 Int2::operator>>(U32 shft) const
{
    return Int2(x >> shft, y >> shft);
}


Float2 lerp(const Float2& a, const Float2& b, F32 t)
{
    return R_LERP(a, b, t);
}


F32 dot(const Float2& a, const Float2& b)
{
    return (a[0] * b[0]) + (a[1] * b[1]); 
}


F32 length(const Float2& a)
{
    return sqrt(dot(a, a));
}


Float2 normalize(const Float2& v)
{
    F32 magnitude = length(v);
    F32 denom = 1.0f / magnitude;
    return v * denom;
}
} // Math
} // Recluse