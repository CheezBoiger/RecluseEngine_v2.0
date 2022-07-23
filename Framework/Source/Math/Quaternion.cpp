// 
#include "Recluse/Math/Quaternion.hpp"

namespace Recluse {


Quaternion Quaternion::operator*(const Quaternion& rh) const
{
    return Quaternion(
        (w * rh.x) + (x * rh.w) + (y * rh.z) - (z * rh.y),
        (w * rh.y) - (x * rh.z) + (y * rh.w) + (z * rh.x),
        (w * rh.z) + (x * rh.y) - (y * rh.x) + (z * rh.w),
        (w * rh.w) - (x * rh.x) - (y * rh.y) - (z * rh.z)
    );
}


Quaternion Quaternion::operator-() const
{
    return Quaternion(-x, -y, -z, -w);
}


Quaternion Quaternion::operator+(const Quaternion& rh) const
{
    return Quaternion(x + rh.x, y + rh.y, z + rh.z, w + rh.w);
}


Quaternion Quaternion::operator-(const Quaternion& rh) const
{
    return Quaternion(x - rh.x, y - rh.y, z - rh.z, w - rh.w);
}


Quaternion Quaternion::operator*(F32 scalar) const
{
    return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}


Quaternion Quaternion::operator+(F32 scalar) const
{
    return Quaternion(x + scalar, y + scalar, z + scalar, w + scalar);
}


Quaternion Quaternion::operator-(F32 scalar) const
{
    return Quaternion(x - scalar, y - scalar, z - scalar, w - scalar);
}


Quaternion Quaternion::operator/(F32 scalar) const
{
    F32 denom = 1.0f / scalar;
    return Quaternion(x * denom, y * denom, z * denom, w * denom);
}


Float3 Quaternion::operator*(const Float3& rh) const
{
    Float3 u(x, y, z);
    F32 s = w;

    return 2.0f * u * dot(u, rh) + rh * (s * s - dot(u, u)) + 2.0f * s * cross(u, rh);
}


Quaternion conjugate(const Quaternion& quat)
{
    return Quaternion(-quat[0], -quat[1], -quat[2], quat[3]);
}


Quaternion inverse(const Quaternion& quat)
{
    Quaternion q    = conjugate(quat);
    F32 n2          = 1.0f / norm2(quat);
    return q * n2;
}


F32 norm2(const Quaternion& quat)
{
    return dot(quat, quat);
}


F32 norm(const Quaternion& quat)
{
    return sqrtf(norm2(quat));
}


Quaternion normalize(const Quaternion& quat)
{
    F32 n = 1.0f / norm(quat);
    return quat * n;
}


F32 dot(const Quaternion& a, const Quaternion& b)
{
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}


Quaternion slerp(const Quaternion& a, const Quaternion& b, F32 t)
{
    F32 d                   = dot(a, b);
    const F32 kThreshold    = 0.9995f;
    Quaternion q;
    Quaternion q1;

    if (fabs(d) > kThreshold) 
    {    
        q = a + (b - a) * t;
        return normalize(q);
    
    }

    q = b;

    if (d < 0.0f) 
    {
        q = -b;
        d = -d;
    
    }

    d           = R_CLAMP(d, -1, 1);

    F32 theta0  = acosf(d);
    F32 theta   = theta0 * t;
    q1          = q - a * d;
    q1          = normalize(q1);

    return a * cosf(theta) + q1 * sinf(theta);
}
} // Recluse