//
#include "Recluse/Math/Vector3.hpp"

namespace Recluse {


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
    return Float3(x / rh.x, y / rh.y);
}


Float3 Float3::operator*(F32 scalar) const
{
    return Float3(x * scalar, y * scalar, z * scalar);
}


Float3 Float3::operator/(F32 scalar) const
{
    return Float3(x / scalar, y / scalar, z / scalar);
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
} // Recluse