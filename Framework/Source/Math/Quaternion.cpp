// 
#include "Recluse/Math/Quaternion.hpp"

namespace Recluse {
namespace Math {

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


Matrix44 quatToMat44(const Quaternion& quat)
{
    F32 xx = quat.x * quat.x;
    F32 yy = quat.y * quat.y;
    F32 zz = quat.z * quat.z;
    F32 xy = quat.x * quat.y;
    F32 wz = quat.w * quat.z;
    F32 xz = quat.x * quat.z;
    F32 wy = quat.w * quat.y;
    F32 yz = quat.y * quat.z;
    F32 wx = quat.w * quat.x;
    return Matrix44
    (
        1.f - 2.f * (yy + zz),  2.f * (xy + wz),        2.f * (xz - wy),        0.f,
        2.f * (xy - wz),        1.f - 2.f * (xx + zz),  2.f * (yz + wx),        0.f,
        2.f * (xz + wy),        2.f * (yz - wx),        1.f - 2.f * (xx + yy),  0.f,
        0.f,                    0.f,                    0.f,                    1.f
    );
}


Quaternion angleAxis(const Float3& axis, F32 radians)
{
    F32 r2  = radians * 0.5f;
    F32 s   = sinf(r2);
    F32 c   = cosf(r2);

    return Quaternion
        (
            axis[0] * s,
            axis[1] * s,
            axis[2] * s,
            c
        ); 
}


Quaternion eulerToQuat(const Float3& euler)
{
    Quaternion q;

    F32 t0 = cosf(euler[2] * 0.5f);
    F32 t1 = sinf(euler[2] * 0.5f);
    F32 t2 = cosf(euler[0] * 0.5f);
    F32 t3 = sinf(euler[0] * 0.5f);
    F32 t4 = cosf(euler[1] * 0.5f);
    F32 t5 = sinf(euler[1] * 0.5f);

    q[0] = t0 * t3 * t4 - t1 * t2 * t5;
    q[1] = t0 * t2 * t5 + t1 * t3 * t4;
    q[2] = t1 * t2 * t4 - t0 * t3 * t5; 
    q[3] = t0 * t2 * t4 + t1 * t3 * t5;

    return q;
}


Float3 quatToEuler(const Quaternion& quat)
{
    Float3 euler;

    F32 y2      = quat[1] * quat[1];
    
    // Roll
    F32 t0      = 2.0f * (quat[3] * quat[0] - quat[1] * quat[2]);
    F32 t1      = 1.0f - 2.0f * (quat[0] * quat[0] + y2);
    
    // Pitch
    F32 t2      = 2.0f * (quat[3] * quat[1] - quat[0] * quat[2]);
    t2          = Math::clamp(t2, -1.0f, 1.0f);
    
    // Yaw
    F32 t3      = 2.0f * (quat[3] * quat[2] + quat[0] * quat[1]);
    F32 t4      = 1.0f - 2.0f * (y2 + quat[2] * quat[2]);
    
    euler[0]    = atan2f(t0, t1);
    euler[1]    = asinf(t2);
    euler[2]    = atan2f(t3, t4);

    return euler;
}
} // Math
} // Recluse