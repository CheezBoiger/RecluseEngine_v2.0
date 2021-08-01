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


Float4 operator+(F32 scalar, const Float4& rh)
{
    return Float4(rh[0] + scalar, rh[1] + scalar, rh[2] + scalar, rh[3] + scalar);
}


Float4 operator-(F32 scalar, const Float4& rh)
{
    return Float4(scalar - rh[0], scalar - rh[1], scalar - rh[2], scalar - rh[3]);
}


Float4 operator*(F32 scalar, const Float4& rh)
{
    return Float4(rh[0] * scalar, rh[1] * scalar, rh[2] * scalar, rh[3] * scalar);
}


Float4 operator/(F32 scalar, const Float4& rh)
{
    F32 denomX = 1.0f / rh[0];
    F32 denomY = 1.0f / rh[1];
    F32 denomZ = 1.0f / rh[2];
    F32 denomW = 1.0f / rh[3];
    return Float4(scalar * denomX, scalar * denomY, scalar * denomZ, scalar * denomW);
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


F32 dot(const Float4& a, const Float4& b)
{
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}


F32 length(const Float4& a)
{
    return sqrtf(dot(a, a));
}


F32 length2(const Float4& a)
{
    return dot(a, a);
}
} // Recluse