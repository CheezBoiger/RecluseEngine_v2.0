//
#include "Recluse/Math/Matrix22.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Math { 


Matrix22::Matrix22
    (
        F32 a00, F32 a01, 
        F32 a10, F32 a11
    )
{
    m[0] = a00; m[1] = a01;
    m[2] = a10; m[3] = a11;
}


Matrix22::Matrix22
    (
        const Float2& row0,
        const Float2& row1
    )
{
    m[0] = row0[0]; m[1] = row0[1];
    m[2] = row1[0]; m[3] = row1[1];
}


F32 Matrix22::get(U32 row, U32 col) const
{
    R_ASSERT(row < 2 && col < 2);
    return m[2 * row + col];
}


F32& Matrix22::get(U32 row, U32 col) 
{
    R_ASSERT(row < 2 && col < 2);
    return m[2 * row + col];
}


Matrix22 Matrix22::operator+(const Matrix22& rh) const
{
    return Matrix22
        (
            m[0] + rh[0], m[1] + rh[1],
            m[2] + rh[2], m[3] + rh[3]
        ); 
}


Matrix22 Matrix22::operator-(const Matrix22& rh) const
{
    return Matrix22
        (
            m[0] - rh[0], m[1] - rh[1],
            m[2] - rh[2], m[3] - rh[3]
        );
}


Matrix22 Matrix22::operator+(F32 scalar) const
{
    return Matrix22
        (
            m[0] + scalar, m[1] + scalar,
            m[2] + scalar, m[3] + scalar
        );
}


Matrix22 Matrix22::operator-(F32 scalar) const
{
    return Matrix22
        (
            m[0] - scalar, m[1] - scalar,
            m[2] - scalar, m[3] - scalar
        );
}


Matrix22 Matrix22::operator*(F32 scalar) const
{
    return Matrix22
        (
            m[0] * scalar, m[1] * scalar,
            m[2] * scalar, m[3] * scalar
        );
}


Matrix22 Matrix22::operator-() const
{
    return Matrix22
        (
            -m[0], -m[1],
            -m[2], -m[3]
        );
}


Matrix22 Matrix22::operator/(F32 scalar) const
{
    F32 denom = 1.f / scalar;
    return Matrix22
        (
            m[0] * denom, m[1] * denom,
            m[2] * denom, m[3] * denom
        );
}


Matrix22 Matrix22::operator*(const Matrix22& rh) const
{
    return Matrix22
        (
            m[0] * rh[0] + m[1] * rh[2], m[0] * rh[1] + m[1] * rh[3],
            m[2] * rh[0] + m[3] * rh[2], m[2] * rh[1] + m[3] * rh[3]
        );
}
} // Math
} // Recluse