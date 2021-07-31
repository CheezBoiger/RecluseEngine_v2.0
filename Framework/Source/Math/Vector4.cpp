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
    F32 denomX = 1.0f / rh.x;
    F32 denomY = 1.0f / rh.y;
    F32 denomZ = 1.0f / rh.z;
    F32 denomW = 1.0f / rh.w;

    return Float4(x * denomX, y * denomY, z * denomZ, w * denomW);
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
    F32 denom = 1.0f / scalar;

    return Float4(x * denom, y * denom, z * denom, w * denom);
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


UInt4 UInt4::operator+(const UInt4& rh) const
{
    return UInt4(x + rh.x, y + rh.y, z + rh.z, w + rh.w);
}


UInt4 UInt4::operator-(const UInt4& rh) const
{
    return UInt4(x - rh.x, y - rh.y, z - rh.z, w - rh.w);
}


UInt4 UInt4::operator*(const UInt4& rh) const
{
    return UInt4(x * rh.x, y * rh.y, z * rh.z, w * rh.w);
}


UInt4 UInt4::operator/(const UInt4& rh) const
{
    return UInt4(x / rh.x, y / rh.y, z / rh.z, w / rh.w);
}


UInt4 UInt4::operator-() const
{
    return UInt4(-x, -y, -z, -w);
}


UInt4 UInt4::operator+(U32 scalar) const
{
    return UInt4(x + scalar, y + scalar, z + scalar, w + scalar);
}
} // Recluse