//
#include "Recluse/Math/Vector4.hpp"


namespace Recluse {


Float4 Float4::operator+(const Float4& rh) const
{
    return Float4(x + rh.x, y + rh.y, z + rh.z, w + rh.w);
}


Float4 Float4::operator-(const Float4& rh) const
{
    return Float4(x - rh.x, y - rh.y, z - rh.z, w - rh.w);
}


Float4 Float4::operator*(const Float4& rh) const
{
    return Float4(x * rh.x, y * rh.y, z * rh.z, w * rh.w);
}


Float4 Float4::operator/(const Float4& rh) const
{
    return Float4(x / rh.x, y / rh.y, z / rh.z, w / rh.w);
}


Float4 Float4::operator+(F32 scalar) const
{
    return Float4(x + scalar, y + scalar, z + scalar, w + scalar);
}


Float4 Float4::operator-(F32 scalar) const
{
    return Float4(x - scalar, y - scalar, z - scalar, w - scalar);
}


Float4 Float4::operator*(F32 scalar) const
{
    return Float4(x * scalar, y * scalar, z * scalar, w * scalar);
}


Float4 Float4::operator/(F32 scalar) const
{
    return Float4(x / scalar, y / scalar, z / scalar, w / scalar);
}


Float4 Float4::operator-() const
{
    return Float4(-x, -y, -z, -w);
}


Float4 Float4::operator==(const Float4& rh) const
{
    return Float4(x == rh.x, y == rh.y, z == rh.z, w == rh.w);
}


Float4 Float4::operator&&(const Float4& rh) const
{
    return Float4(x && rh.x, y && rh.y, z && rh.z, w && rh.w);
}


Float4 Float4::operator||(const Float4& rh) const
{
    return Float4(x || rh.x, y || rh.y, z || rh.z, w || rh.w);
}


Float4 Float4::operator<(const Float4& rh) const
{
    return Float4(x < rh.x, y < rh.y, z < rh.z, w < rh.w);
}


Float4 Float4::operator>(const Float4& rh) const
{
    return Float4(x > rh.x, y > rh.y, z > rh.z, w > rh.w);
}


Float4 Float4::operator>=(const Float4& rh) const
{
    return Float4(x >= rh.x, y >= rh.y, z >= rh.z, w >= rh.w);
}


Float4 Float4::operator<=(const Float4& rh) const
{
    return Float4(x <= rh.x, y <= rh.y, z <= rh.z, w < rh.w);
}
} // Recluse