//
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {
namespace Math {

Float3 Float3::operator+(const Float3& rh) const
{
    return Float3(x + rh.x, y + rh.y);
}


Float3 Float3::operator-(const Float3& rh) const
{
    return Float3(x - rh.x, y - rh.y);
}


Float3 Float3::operator-() const
{
    return Float3(-x, -y, -z);
}


Float3 Float3::operator*(const Float3& rh) const
{
    return Float3(x * rh.x, y * rh.y, z * rh.z);
}


Float3 Float3::operator/(const Float3& rh) const
{
    F32 denomX = 1.0f / rh.x;
    F32 denomY = 1.0f / rh.y;
    F32 denomZ = 1.0f / rh.z;
    return Float3(x * denomX, y * denomY, z * denomZ);
}


Float3 Float3::operator*(F32 scalar) const
{
    return Float3(x * scalar, y * scalar, z * scalar);
}


Float3 Float3::operator/(F32 scalar) const
{
    F32 denom = 1.0f / scalar;
    return Float3(x * denom, y * denom, z * denom);
}


Float3 Float3::operator+(F32 scalar) const
{
    return Float3(x + scalar, y + scalar, z + scalar);
}


Float3 Float3::operator-(F32 scalar) const
{
    return Float3(x - scalar, y - scalar, z - scalar);
}


Bool3 Float3::operator==(const Float3& rh) const
{
    return Bool3(x == rh.x, y == rh.y, z == rh.z);
}


Bool3 Float3::operator!=(const Float3& rh) const
{
    return !(*this == rh);
}


Bool3 Float3::operator&&(const Float3& rh) const
{
    return Bool3(x && rh.x, y && rh.y, z && rh.z);
}


Bool3 Float3::operator||(const Float3& rh) const
{
    return Bool3(x || rh.x, y || rh.y, z || rh.z);
}


Bool3 Float3::operator<(const Float3& rh) const
{
    return Bool3(x < rh.x, y < rh.y, z < rh.z);
}


Bool3 Float3::operator>(const Float3& rh) const
{
    return Bool3(x > rh.x, y > rh.y, z > rh.z);
}


Bool3 Float3::operator>=(const Float3& rh) const
{
    return Bool3(x >= rh.x, y >= rh.y, z >= rh.z);
}


Bool3 Float3::operator<=(const Float3& rh) const
{
    return Bool3(x <= rh.x, y <= rh.y, z <= rh.z);
}


Float3 operator*(F32 scalar, const Float3& rh)
{
    return rh * scalar;
}

Float3 operator+(F32 scalar, const Float3& rh)
{
    return rh + scalar;
}


Float3 operator-(F32 scalar, const Float3& rh)
{
    return Float3(scalar - rh.x, scalar - rh.y, scalar - rh.z);
}


Float3 operator/(F32 scalar, const Float3& rh)
{
    F32 denomX = 1.0f / rh[0];
    F32 denomY = 1.0f / rh[1];
    F32 denomZ = 1.0f / rh[2];

    return Float3(scalar * denomX, scalar * denomY, scalar * denomZ);
}


Bool3 Float3::operator==(F32 scalar) const
{
    return Bool3(x == scalar, y == scalar, z == scalar);
}


Bool3 Float3::operator<(F32 scalar) const
{
    return Bool3(x < scalar, y < scalar, z < scalar);
}


Bool3 Float3::operator>(F32 scalar) const
{
    return Bool3(x > scalar, y > scalar, z > scalar);
}


Bool3 Float3::operator<=(F32 scalar) const
{
    return Bool3(x <= scalar, y <= scalar, z <= scalar);
}


Bool3 Float3::operator>=(F32 scalar) const
{
    return Bool3(x >= scalar, y >= scalar, z >= scalar);
}


F32 dot(const Float3& lh, const Float3& rh)
{
    return ((lh[0] * rh[0]) + (lh[1] * rh[1]) + (lh[2] * rh[2]));
}


U32 dot(const Int3& lh, const Int3& rh)
{
    return ((lh[0] * rh[0]) + (lh[1] * rh[1]) + (lh[2] * rh[2]));
}


F32 length(const Float3& v)
{
    return sqrtf(dot(v, v));
}


F32 length2(const Float3& v)
{
    return dot(v, v);
}


Float3 normalize(const Float3& v)
{
    F32 magnitude = length(v);
    F32 denom = 1.0f / magnitude;
    return v * denom;
}

Bool any(const Float3& a)
{
    return (a[0] != 0.f) || (a[1] != 0.f) || (a[2] != 0.f);
}


Bool all(const Float3& a)
{
    return (a[0] != 0.f) && (a[1] != 0.f) && (a[2] != 0.f);
}


Float3 cross(const Float3& lh, const Float3& rh)
{
    return {
            lh.y * rh.z - lh.z * rh.y, 
            lh.z * rh.x - lh.x * rh.z,
            lh.x * rh.y - lh.y * rh.x
    };
}


Float3 reflect(const Float3& incidence, const Float3& normal)
{
    return  incidence - 2.0f * dot(normal, incidence) * normal;
}


Float3 refract(const Float3& incidence, const Float3& normal, F32 eta)
{
    F32 NoI = dot(normal, incidence);
    F32 NoI2 = NoI * NoI;
    F32 eta2 = eta * eta;
    F32 k = 1.0f - eta2 * (1.0f - NoI2);
    Float3 R{0.f, 0.f, 0.f};
    if (k >= 0.f)
    {
        R = eta * incidence - (eta * NoI + sqrtf(k)) * normal;
    }
    return R;
}


Color3 floatToColor(const Float3& color)
{
    const F32 maxChroma = 255.f;
    const U8 R = static_cast<U8>(color.r * maxChroma);
    const U8 G = static_cast<U8>(color.g * maxChroma);
    const U8 B = static_cast<U8>(color.b * maxChroma);
    return Color3(R, G, B);
}


Float3 colorToFloat(const Color3& color)
{
    const F32 inv = 1.f / 255.f;
    const F32 R = static_cast<F32>(color.r);
    const F32 G = static_cast<F32>(color.g);
    const F32 B = static_cast<F32>(color.b);
    return Float3(R * inv, G * inv, B * inv);
}


F32 dist(const Float3& p0, const Float3& p1)
{
    return length(p1 - p0);
}


U32 dist(const UByte3& p0, const UByte3& p1)
{
    const Int3 ip0 = Int3(p0[0], p0[1], p0[2]);
    const Int3 ip1 = Int3(p1[0], p1[1], p1[2]);
    const Int3 q = ip1 - ip0;
    return isqrt(dot(q, q));
}
} // Math
} // Recluse