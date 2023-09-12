//
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {
namespace Math {

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


Bool4 Float4::operator==(const Float4& rh) const
{
    return Bool4(x == rh.x, y == rh.y, z == rh.z, w == rh.w);
}

Bool4 Float4::operator!=(const Float4& rh) const
{
    return !(*this == rh);
}


Bool4 Float4::operator&&(const Float4& rh) const
{
    return Bool4(x && rh.x, y && rh.y, z && rh.z, w && rh.w);
}


Bool4 Float4::operator||(const Float4& rh) const
{
    return Bool4(x || rh.x, y || rh.y, z || rh.z, w || rh.w);
}


Bool4 Float4::operator<(const Float4& rh) const
{
    return Bool4(x < rh.x, y < rh.y, z < rh.z, w < rh.w);
}


Bool4 Float4::operator>(const Float4& rh) const
{
    return Bool4(x > rh.x, y > rh.y, z > rh.z, w > rh.w);
}


Bool4 Float4::operator>=(const Float4& rh) const
{
    return Bool4(x >= rh.x, y >= rh.y, z >= rh.z, w >= rh.w);
}


Bool4 Float4::operator<=(const Float4& rh) const
{
    return Bool4(x <= rh.x, y <= rh.y, z <= rh.z, w < rh.w);
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


Bool4 Float4::operator==(F32 scalar) const
{
    return Bool4(x == scalar, y == scalar, z == scalar, w == scalar);
}


Bool4 Float4::operator<(F32 scalar) const
{
    return Bool4(x < scalar, y < scalar, z < scalar, w < scalar);
}


Bool4 Float4::operator>(F32 scalar) const
{
    return Bool4(x > scalar, y > scalar, z > scalar, w > scalar);
}


Bool4 Float4::operator<=(F32 scalar) const
{
    return Bool4(x <= scalar, y <= scalar, z <= scalar, w <= scalar);
}


Bool4 Float4::operator>=(F32 scalar) const
{
    return Bool4(x >= scalar, y >= scalar, z >= scalar, w >= scalar);
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


Float4 operator*(const Float4& lh, const Matrix44& rh)
{
    Float4 res;

    res[0] = lh[0] * rh[0] + lh[1] * rh[1] + lh[2] * rh[2] + lh[3] * rh[3];
    res[1] = lh[0] * rh[4] + lh[1] * rh[5] + lh[2] * rh[6] + lh[3] * rh[7];
    res[2] = lh[0] * rh[8] + lh[1] * rh[9] + lh[2] * rh[10] + lh[3] * rh[11];
    res[3] = lh[0] * rh[12] + lh[1] * rh[13] + lh[2] * rh[14] + lh[3] * rh[15];

    return res;
}


Float4 normalize(const Float4& lh)
{
    F32 magnitude = length(lh);
    F32 denom = 1.0f / magnitude;
    return lh * denom;
}


Bool any(const Float4& a)
{
    return (a[0] != 0.f) || (a[1] != 0.f) || (a[2] != 0.f) || (a[3] != 0.f);
}


Bool all(const Float4& a)
{
    return (a[0] != 0.f) && (a[1] != 0.f) && (a[2] != 0.f) && (a[3] != 0.f);
}


Float4 colorToFloat(const Color4& color)
{
    F32 inv = 1.0f / 255.f;
    F32 R = static_cast<F32>(color.r);
    F32 G = static_cast<F32>(color.g);
    F32 B = static_cast<F32>(color.b);
    F32 A = static_cast<F32>(color.a);
    return Float4(R * inv, G * inv, B * inv, A * inv);
}


Color4 floatToColor(const Float4& color)
{
    F32 maxChroma = 255.f;
    U8 R = static_cast<U8>(color.r * maxChroma);
    U8 G = static_cast<U8>(color.g * maxChroma);
    U8 B = static_cast<U8>(color.b * maxChroma);
    U8 A = static_cast<U8>(color.a * maxChroma);
    return Color4(R, G, B, A);
}
} // Math
} // Recluse