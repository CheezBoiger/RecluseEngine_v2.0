//
#include "Recluse/Math/Vector2.hpp"

namespace Recluse {


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
} // Recluse