//
#include "Recluse/Math/Vector3.hpp"

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


Float3 Float3::operator==(const Float3& rh) const
{
    return Float3(x == rh.x, y == rh.y, z == rh.z);
}


Float3 Float3::operator&&(const Float3& rh) const
{
    return Float3(x && rh.x, y && rh.y, z && rh.z);
}


Float3 Float3::operator||(const Float3& rh) const
{
    return Float3(x || rh.x, y || rh.y, z || rh.z);
}


Float3 Float3::operator<(const Float3& rh) const
{
    return Float3(x < rh.x, y < rh.y, z < rh.z);
}


Float3 Float3::operator>(const Float3& rh) const
{
    return Float3(x > rh.x, y > rh.y, z > rh.z);
}


Float3 Float3::operator>=(const Float3& rh) const
{
    return Float3(x >= rh.x, y >= rh.y, z >= rh.z);
}


Float3 Float3::operator<=(const Float3& rh) const
{
    return Float3(x <= rh.x, y <= rh.y, z <= rh.z);
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


F32 dot(const Float3& lh, const Float3& rh)
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
    return v / magnitude;
}


Float3 cross(const Float3& lh, const Float3& rh)
{
    return {
            lh.y * rh.z - lh.z * rh.y, 
            lh.z * rh.x - lh.x * rh.z,
            lh.x * rh.y - lh.y * rh.x
    };
}


Float3 lerp(const Float3& a, const Float3& b, F32 t)
{
    return R_LERP(a, b, t);
}
} // Math
} // Recluse