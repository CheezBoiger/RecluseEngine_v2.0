//
#include "Recluse/Math/Matrix33.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Math {

Matrix33::Matrix33
                (
                    F32 a00, F32 a01, F32 a02,
                    F32 a10, F32 a11, F32 a12,
                    F32 a20, F32 a21, F32 a22
                )
{
    m[0] = a00;
    m[1] = a01;
    m[2] = a02;
    m[3] = a10;
    m[4] = a11;
    m[5] = a12;
    m[6] = a20;
    m[7] = a21;
    m[8] = a22;
}


Matrix33::Matrix33
                (
                    const Float3& row0,
                    const Float3& row1,
                    const Float3& row2
                )
{
    m[0] = row0[0];
    m[1] = row0[1];
    m[2] = row0[2];
    m[3] = row1[0];
    m[4] = row1[1];
    m[5] = row1[2];
    m[6] = row2[0];
    m[7] = row2[1];
    m[8] = row2[2];
}


Matrix33 Matrix33::operator+(const Matrix33& rh) const
{
    Matrix33 ans;

    ans[0] = m[0] + rh[0];
    ans[1] = m[1] + rh[1];
    ans[2] = m[2] + rh[2];
    ans[3] = m[3] + rh[3];
    ans[4] = m[4] + rh[4];
    ans[5] = m[5] + rh[5];
    ans[6] = m[6] + rh[6];
    ans[7] = m[7] + rh[7];
    ans[8] = m[8] + rh[8];

    return ans;
}


Matrix33 Matrix33::operator-(const Matrix33& rh) const
{
    Matrix33 ans;

    ans[0] = m[0] - rh[0];
    ans[1] = m[1] - rh[1];
    ans[2] = m[2] - rh[2];
    ans[3] = m[3] - rh[3];
    ans[4] = m[4] - rh[4];
    ans[5] = m[5] - rh[5];
    ans[6] = m[6] - rh[6];
    ans[7] = m[7] - rh[7];
    ans[8] = m[8] - rh[8];

    return ans;
}


void Matrix33::operator+=(const Matrix33& rh)
{
    m[0] += rh[0];
    m[1] += rh[1];
    m[2] += rh[2];
    m[3] += rh[3];
    m[4] += rh[4];
    m[5] += rh[5];
    m[6] += rh[6];
    m[7] += rh[7];
    m[8] += rh[8];
}


void Matrix33::operator-=(const Matrix33& rh)
{
    m[0] -= rh[0];
    m[1] -= rh[1];
    m[2] -= rh[2];
    m[3] -= rh[3];
    m[4] -= rh[4];
    m[5] -= rh[5];
    m[6] -= rh[6];
    m[7] -= rh[7];
    m[8] -= rh[8];
}


void Matrix33::operator*=(F32 scalar)
{
    m[0] *= scalar;
    m[1] *= scalar;
    m[2] *= scalar;
    m[3] *= scalar;
    m[4] *= scalar;
    m[5] *= scalar;
    m[6] *= scalar;
    m[7] *= scalar;
    m[8] *= scalar;
}


void Matrix33::operator+=(F32 scalar)
{
    m[0] += scalar;
    m[1] += scalar;
    m[2] += scalar;
    m[3] += scalar;
    m[4] += scalar;
    m[5] += scalar;
    m[6] += scalar;
    m[7] += scalar;
    m[8] += scalar;    
}


void Matrix33::operator-=(F32 scalar)
{
    m[0] -= scalar;
    m[1] -= scalar;
    m[2] -= scalar;
    m[3] -= scalar;
    m[4] -= scalar;
    m[5] -= scalar;
    m[6] -= scalar;
    m[7] -= scalar;
    m[8] -= scalar;    
}


void Matrix33::operator/=(F32 scalar)
{
    m[0] /= scalar;
    m[1] /= scalar;
    m[2] /= scalar;
    m[3] /= scalar;
    m[4] /= scalar;
    m[5] /= scalar;
    m[6] /= scalar;
    m[7] /= scalar;
    m[8] /= scalar;
}


Matrix33 Matrix33::operator+(F32 scalar) const
{
    Matrix33 ans;
    ans[0] = m[0] + scalar;
    ans[1] = m[1] + scalar;
    ans[2] = m[2] + scalar;
    ans[3] = m[3] + scalar;
    ans[4] = m[4] + scalar;
    ans[5] = m[5] + scalar;
    ans[6] = m[6] + scalar;
    ans[7] = m[6] + scalar;
    ans[8] = m[8] + scalar;
    return ans;
}


Matrix33 Matrix33::operator-(F32 scalar) const
{
    Matrix33 ans;
    ans[0] = m[0] - scalar;
    ans[1] = m[1] - scalar;
    ans[2] = m[2] - scalar;
    ans[3] = m[3] - scalar;
    ans[4] = m[4] - scalar;
    ans[5] = m[5] - scalar;
    ans[6] = m[6] - scalar;
    ans[7] = m[6] - scalar;
    ans[8] = m[8] - scalar;
    return ans;
}


Matrix33 Matrix33::operator*(F32 scalar) const
{
    Matrix33 ans;
    ans[0] = m[0] * scalar;
    ans[1] = m[1] * scalar;
    ans[2] = m[2] * scalar;
    ans[3] = m[3] * scalar;
    ans[4] = m[4] * scalar;
    ans[5] = m[5] * scalar;
    ans[6] = m[6] * scalar;
    ans[7] = m[6] * scalar;
    ans[8] = m[8] * scalar;
    return ans;
}


Matrix33 Matrix33::operator/(F32 scalar) const
{
    Matrix33 ans;
    ans[0] = m[0] / scalar;
    ans[1] = m[1] / scalar;
    ans[2] = m[2] / scalar;
    ans[3] = m[3] / scalar;
    ans[4] = m[4] / scalar;
    ans[5] = m[5] / scalar;
    ans[6] = m[6] / scalar;
    ans[7] = m[6] / scalar;
    ans[8] = m[8] / scalar;
    return ans;
}


F32 Matrix33::get(U32 row, U32 col) const
{
    R_ASSERT(row < 3 && col < 3);
    return m[3u * row + col];
}


F32& Matrix33::get(U32 row, U32 col) 
{   
    R_ASSERT(row < 3 && col < 3);
    return m[3u * row + col];
}


F32 determinant(const Matrix33& m)
{
    return  (m[0] * m[4] * m[8]) + 
            (m[1] * m[5] * m[6]) + 
            (m[2] * m[3] * m[7]) - 
            (m[2] * m[4] * m[6]) -
            (m[1] * m[3] * m[8]) -
            (m[0] * m[5] * m[7]);
}


Matrix33 transpose(const Matrix33& m)
{
    Matrix33 t = m;
    t[1] = m[3];
    t[2] = m[6];
    t[5] = m[7];
    t[7] = m[5];
    t[6] = m[2];
    t[3] = m[1];
    return t;
}


Matrix33 Matrix33::identity()
{
    return Matrix33
            (
                1.f, 0.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 0.f, 1.f 
            );
}
} // Math
} // Recluse