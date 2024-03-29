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


Float2 Float2::operator==(F32 scalar) const
{
    return Float2(x == scalar, y == scalar);
}


Float2 Float2::operator<(F32 scalar) const
{
    return Float2(x < scalar, y < scalar);
}


Float2 Float2::operator>(F32 scalar) const
{
    return Float2(x > scalar, y > scalar);
}


Float2 Float2::operator>=(F32 scalar) const
{
    return Float2(x >= scalar, y >= scalar);
}


Float2 Float2::operator<=(F32 scalar) const
{
    return Float2(x <= scalar, y <= scalar);
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


F32 dot(const Float2& a, const Float2& b)
{
    return (a[0] * b[0]) + (a[1] * b[1]); 
}


F32 length(const Float2& a)
{
    return sqrt(dot(a, a));
}


Float2 operator*(F32 scalar, const Float2& rh)
{
    return Float2(rh[0] * scalar, rh[1] * scalar);
}


Float2 operator+(F32 scalar, const Float2& rh)
{
    return Float2(scalar + rh[0], scalar + rh[1]); 
}


Float2 operator-(F32 scalar, const Float2& rh)
{
    return Float2(scalar - rh[0], scalar - rh[1]);
}


Float2 operator/(F32 scalar, const Float2& rh)
{
    F32 denX = 1.0f / rh[0];
    F32 denY = 1.0f / rh[1];
    return Float2(scalar * denX, scalar * denY);
}


Float2 normalize(const Float2& v)
{
    F32 magnitude = length(v);
    F32 denom = 1.0f / magnitude;
    return v * denom;
}


Float2 reflect(const Float2& incidence, const Float2& normal)
{
    return incidence - 2.0 * dot(incidence, normal) * normal;
}


Bool any(const Float2& a)
{
    return (a[0] != 0.f) || (a[1] != 0.f);
}


Bool all(const Float2& a)
{
    return (a[0] != 0.f) && (a[1] != 0.f);
}


Float2 refract(const Float2& incidence, const Float2& normal, F32 eta)
{
    F32 NoI = dot(normal, incidence);
    F32 NoI2 = NoI * NoI;
    F32 eta2 = eta * eta;
    F32 k = 1.0f - eta2 * (1.0f - NoI2);
    Float2 R{0.f, 0.f};
    if (k >= 0.f)
    {
        R = eta * incidence - (eta * NoI + sqrtf(k)) * normal;
    }
    return R;
}
} // Math
} // Recluse