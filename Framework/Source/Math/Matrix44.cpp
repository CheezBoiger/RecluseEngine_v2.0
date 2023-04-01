//
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Matrix43.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Math {

Matrix44::Matrix44
                (
                    F32 a00, F32 a01, F32 a02, F32 a03,
                    F32 a10, F32 a11, F32 a12, F32 a13,
                    F32 a20, F32 a21, F32 a22, F32 a23,
                    F32 a30, F32 a31, F32 a32, F32 a33
                )
{
    m[0]  = a00;
    m[1]  = a01;
    m[2]  = a02;
    m[3]  = a03;
    m[4]  = a10;
    m[5]  = a11;
    m[6]  = a12;
    m[7]  = a13;
    m[8]  = a20;
    m[9]  = a21;
    m[10] = a22;
    m[11] = a23;
    m[12] = a30;
    m[13] = a31;
    m[14] = a32;
    m[15] = a33;
}


Matrix44::Matrix44
            (
                const Float4& row0,
                const Float4& row1,
                const Float4& row2,
                const Float4& row3
            )
{
    this->row0 = *(__m128*)&row0.x;
    this->row1 = *(__m128*)&row1.x;
    this->row2 = *(__m128*)&row2.x;
    this->row3 = *(__m128*)&row3.x;
}


Matrix44 rotate(const Matrix44& lh, const Float3& ax, F32 radians)
{
    F32 cosine          = cosf(radians);
    F32 sine            = sinf(radians);
    F32 oneMinusCosine  = 1.0f - cosine;

    Float3 axis         = normalize(ax);
    
    // Solves Rodrigues' Rotation Formula, applied to the final result. 
    Matrix44 rotator = 
        {
            cosine + (axis.x * axis.x) * oneMinusCosine,      
            oneMinusCosine * axis.y * axis.x + axis.z * sine, 
            axis.z * axis.x * oneMinusCosine - axis.y * sine, 
            0,
            axis.x * axis.y * oneMinusCosine - axis.z * sine, 
            cosine + (axis.y * axis.y) * oneMinusCosine,      
            axis.z * axis.y * oneMinusCosine + axis.x * sine, 
            0,
            axis.x * axis.z * oneMinusCosine + axis.y * sine, 
            axis.y * axis.z * oneMinusCosine - axis.x * sine, 
            cosine + (axis.z * axis.z) * oneMinusCosine,      
            0,
            0,                                                
            0,                                                
            0,                                                
            1        
        };
    
    return lh * rotator;
}


Matrix44 scale(const Matrix44& lh, const Float4& factor)
{
    Matrix44 ans = lh;

    ans[0]  *= factor.x;
    ans[5]  *= factor.y;
    ans[10] *= factor.z;
    ans[15] *= factor.w;

    return ans;
}


Matrix44 translate(const Matrix44& lh, const Float3& rh)
{
    Matrix44 ans = lh;

    ans[12] += lh[0]  * rh.x;
    ans[13] += lh[5]  * rh.y;
    ans[14] += lh[10] * rh.z;

    return ans;
}


Matrix44 transpose(const Matrix44& lh)
{
    Matrix44 ans    = lh;

    ans[1]          = lh[4];
    ans[2]          = lh[8];
    ans[3]          = lh[12];
    ans[7]          = lh[13];
    ans[11]         = lh[14];
    ans[12]         = lh[3];
    ans[13]         = lh[7];
    ans[14]         = lh[11];
    ans[8]          = lh[2];
    ans[4]          = lh[1];

    return ans;
}


Matrix44 adjugate(const Matrix44& lh)
{
    Matrix44 cM;
    // Handles the case of finding our Transpose-Cofactor matrix.
    // This will then be used to locate our inverse.
    //
    cM[0] =     lh[5] * lh[10] * lh[15] + 
                lh[6] * lh[11] * lh[13] +
                lh[7] * lh[9]  * lh[14] -
                lh[7] * lh[10] * lh[13] -
                lh[6] * lh[9]  * lh[15] -
                lh[5] * lh[11] * lh[14];
    cM[1] = -(  lh[1] * lh[10] * lh[15] +
                lh[2] * lh[11] * lh[13] +
                lh[3] * lh[9]  * lh[14] -
                lh[3] * lh[10] * lh[13] -
                lh[2] * lh[9]  * lh[15] -
                lh[1] * lh[11] * lh[14] );
    cM[2] =     lh[1] * lh[6]  * lh[15] +
                lh[2] * lh[7]  * lh[13] +
                lh[3] * lh[5]  * lh[14] -
                lh[3] * lh[6]  * lh[13] -
                lh[2] * lh[5]  * lh[15] -
                lh[1] * lh[7]  * lh[14];
    cM[3] = -(  lh[1] * lh[6]  * lh[11] +
                lh[2] * lh[7]  * lh[9]  +
                lh[3] * lh[5]  * lh[10] -
                lh[3] * lh[6]  * lh[9]  -
                lh[2] * lh[5]  * lh[11] -
                lh[1] * lh[7]  * lh[10] );
    cM[4] = -(  lh[4] * lh[10] * lh[15] +
                lh[6] * lh[11] * lh[12] +
                lh[7] * lh[8]  * lh[14] -
                lh[7] * lh[10] * lh[12] -
                lh[6] * lh[8]  * lh[15] -
                lh[4] * lh[11] * lh[14] ); 
    cM[5] =     lh[0] * lh[10] * lh[15] + 
                lh[2] * lh[11] * lh[12] +
                lh[3] * lh[8]  * lh[14] -
                lh[3] * lh[10] * lh[12] -
                lh[2] * lh[8]  * lh[15] -
                lh[0] * lh[11] * lh[14];
    cM[6] = -(  lh[0] * lh[6]  * lh[15] +
                lh[2] * lh[7]  * lh[12] +
                lh[3] * lh[4]  * lh[14] -
                lh[3] * lh[6]  * lh[12] -
                lh[2] * lh[4]  * lh[15] -
                lh[0] * lh[7]  * lh[14] );
    cM[7] =     lh[0] * lh[6]  * lh[11] +
                lh[2] * lh[7]  * lh[8]  +
                lh[3] * lh[4]  * lh[10] -
                lh[3] * lh[6]  * lh[8]  -
                lh[2] * lh[4]  * lh[11] -
                lh[0] * lh[7]  * lh[10];
    cM[8] =     lh[4] * lh[9]  * lh[15] +
                lh[5] * lh[11] * lh[12] +
                lh[7] * lh[8]  * lh[13] -
                lh[7] * lh[9]  * lh[12] -
                lh[5] * lh[8]  * lh[15] -
                lh[4] * lh[11] * lh[13];
    cM[9] = -(  lh[0] * lh[9]  * lh[15] +
                lh[1] * lh[11] * lh[12] +
                lh[3] * lh[8]  * lh[13] -
                lh[3] * lh[9]  * lh[12] -
                lh[1] * lh[8]  * lh[15] -
                lh[0] * lh[11] * lh[13] );
    cM[10] =    lh[0] * lh[5]  * lh[15] +
                lh[1] * lh[7]  * lh[12] +
                lh[3] * lh[4]  * lh[13] -
                lh[3] * lh[5]  * lh[12] -
                lh[1] * lh[4]  * lh[15] -
                lh[0] * lh[7]  * lh[13];
    cM[11] = -( lh[0] * lh[5]  * lh[11] +
                lh[1] * lh[7]  * lh[8]  +
                lh[3] * lh[4]  * lh[9]  -
                lh[3] * lh[5]  * lh[8]  -
                lh[1] * lh[4]  * lh[11] -
                lh[0] * lh[7]  * lh[9] );
    cM[12] = -( lh[4] * lh[9]  * lh[14] +
                lh[5] * lh[10] * lh[12] +
                lh[6] * lh[8]  * lh[13] -
                lh[6] * lh[9]  * lh[12] -
                lh[5] * lh[8]  * lh[14] -
                lh[4] * lh[10] * lh[13] ); 
    cM[13] =    lh[0] * lh[9]  * lh[14] +
                lh[1] * lh[10] * lh[12] +
                lh[2] * lh[8]  * lh[13] -
                lh[2] * lh[9]  * lh[12] -
                lh[1] * lh[8]  * lh[14] -
                lh[0] * lh[10] * lh[13];
    cM[14] = -( lh[0] * lh[5]  * lh[14] +
                lh[1] * lh[6]  * lh[12] +
                lh[2] * lh[4]  * lh[13] -
                lh[2] * lh[5]  * lh[12] -
                lh[1] * lh[4]  * lh[14] -
                lh[0] * lh[6]  * lh[13] );
    cM[15] =    lh[0] * lh[5]  * lh[10] +
                lh[1] * lh[6]  * lh[8]  +
                lh[2] * lh[4]  * lh[9]  -
                lh[2] * lh[5]  * lh[8]  -
                lh[1] * lh[4]  * lh[10] -
                lh[0] * lh[6]  * lh[9];

    return cM;
}

F32 determinant(const Matrix44& lh)
{
    return lh[0] * (    lh[5] * (lh[10] * lh[15] - lh[11] * lh[14]) -
                        lh[6] * (lh[9] * lh[15] - lh[11] * lh[13]) +
                        lh[7] * (lh[9] * lh[14] - lh[10] * lh[13])
                    ) -
          lh[1] * (     lh[4] * (lh[10] * lh[15] - lh[11] * lh[14]) -
                        lh[6] * (lh[8] * lh[15] - lh[11] * lh[12]) +
                        lh[7] * (lh[8] * lh[14] - lh[10] * lh[12])
                    ) +
          lh[2] * (     lh[4] * (lh[9] * lh[15] - lh[11] * lh[13]) -
                        lh[5] * (lh[8] * lh[15] - lh[11] * lh[12]) +
                        lh[7] * (lh[8] * lh[13] - lh[9] * lh[12])
                    ) -
          lh[3] * (     lh[4] * (lh[9] * lh[14] - lh[10] * lh[13]) -
                        lh[5] * (lh[8] * lh[14] - lh[10] * lh[12]) +
                        lh[6] * (lh[8] * lh[13] - lh[9] * lh[12]) );
}

Matrix44 inverse(const Matrix44& lh)
{
    F32 det         = determinant(lh);
    F32 denom       = 0.f;
    if (det == 0.f)
        return Matrix44::identity();

    Matrix44 adj = adjugate(lh);

    denom = 1.f / det;
    return adj * denom;
}


Matrix44 Matrix44::identity()
{
    return Matrix44
                (
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
                );
}


Matrix44 Matrix44::operator*(const Matrix44& rh) const
{
    Matrix44 ans;

    ans[0]  = m[0]  * rh[0] + m[1]  * rh[4] + m[2]  * rh[8]  + m[3]  * rh[12];
    ans[1]  = m[0]  * rh[1] + m[1]  * rh[5] + m[2]  * rh[9]  + m[3]  * rh[13];
    ans[2]  = m[0]  * rh[2] + m[1]  * rh[6] + m[2]  * rh[10] + m[3]  * rh[14];
    ans[3]  = m[0]  * rh[3] + m[1]  * rh[7] + m[2]  * rh[11] + m[3]  * rh[15];
    
    ans[4]  = m[4]  * rh[0] + m[5]  * rh[4] + m[6]  * rh[8]  + m[7]  * rh[12];
    ans[5]  = m[4]  * rh[1] + m[5]  * rh[5] + m[6]  * rh[9]  + m[7]  * rh[13];
    ans[6]  = m[4]  * rh[2] + m[5]  * rh[6] + m[6]  * rh[10] + m[7]  * rh[14];
    ans[7]  = m[4]  * rh[3] + m[5]  * rh[7] + m[6]  * rh[11] + m[7]  * rh[15];

    ans[8]  = m[8]  * rh[0] + m[9]  * rh[4] + m[10] * rh[8]  + m[11] * rh[12];
    ans[9]  = m[8]  * rh[1] + m[9]  * rh[5] + m[10] * rh[9]  + m[11] * rh[13];
    ans[10] = m[8]  * rh[2] + m[9]  * rh[6] + m[10] * rh[10] + m[11] * rh[14];
    ans[11] = m[8]  * rh[3] + m[9]  * rh[7] + m[10] * rh[11] + m[11] * rh[15];

    ans[12] = m[12] * rh[0] + m[13] * rh[4] + m[14] * rh[8]  + m[15] * rh[12];
    ans[13] = m[12] * rh[1] + m[13] * rh[5] + m[14] * rh[9]  + m[15] * rh[13];
    ans[14] = m[12] * rh[2] + m[13] * rh[6] + m[14] * rh[10] + m[15] * rh[14];
    ans[15] = m[12] * rh[3] + m[13] * rh[7] + m[14] * rh[11] + m[15] * rh[15];

    return ans;
}


Matrix44 Matrix44::operator*(F32 scalar) const
{
    Matrix44 ans;
 
    ans[0]  = m[0]  * scalar;
    ans[1]  = m[1]  * scalar;
    ans[2]  = m[2]  * scalar;
    ans[3]  = m[3]  * scalar;
    ans[4]  = m[4]  * scalar;
    ans[5]  = m[5]  * scalar;
    ans[6]  = m[6]  * scalar;
    ans[7]  = m[7]  * scalar;
    ans[8]  = m[8]  * scalar;
    ans[9]  = m[9]  * scalar;
    ans[10] = m[10] * scalar;
    ans[11] = m[11] * scalar;
    ans[12] = m[12] * scalar;
    ans[13] = m[13] * scalar;
    ans[14] = m[14] * scalar;
    ans[15] = m[15] * scalar;

    return ans;
}


Matrix44 Matrix44::operator/(F32 scalar) const
{
    Matrix44 ans;;
    F32 denom = 1.f / scalar;

    ans[0]  = m[0]  * denom;
    ans[1]  = m[1]  * denom;
    ans[2]  = m[2]  * denom;
    ans[3]  = m[3]  * denom;
    ans[4]  = m[4]  * denom;
    ans[5]  = m[5]  * denom;
    ans[6]  = m[6]  * denom;
    ans[7]  = m[7]  * denom;
    ans[8]  = m[8]  * denom;
    ans[9]  = m[9]  * denom;
    ans[10] = m[10] * denom;
    ans[11] = m[11] * denom;
    ans[12] = m[12] * denom;
    ans[13] = m[13] * denom;
    ans[14] = m[14] * denom;
    ans[15] = m[15] * denom;
 
   return ans;
}


Matrix44 Matrix44::operator+(F32 scalar) const
{
    Matrix44 ans;

    ans[0] =  m[0]  + scalar;
    ans[1] =  m[1]  + scalar;
    ans[2] =  m[2]  + scalar;
    ans[3] =  m[3]  + scalar;
    ans[4] =  m[4]  + scalar;
    ans[5] =  m[5]  + scalar;
    ans[6] =  m[6]  + scalar;
    ans[7] =  m[7]  + scalar;
    ans[8] =  m[8]  + scalar;
    ans[9] =  m[9]  + scalar;
    ans[10] = m[10] + scalar;
    ans[11] = m[11] + scalar;
    ans[12] = m[12] + scalar;
    ans[13] = m[13] + scalar;
    ans[14] = m[14] + scalar;
    ans[15] = m[15] + scalar;
    
    return ans;
}


Matrix44 Matrix44::operator+(const Matrix44& rh) const
{
    Matrix44 ans;
    
    ans[0] =  m[0]  + rh[0];
    ans[1] =  m[1]  + rh[1];
    ans[2] =  m[2]  + rh[2];
    ans[3] =  m[3]  + rh[3];
    ans[4] =  m[4]  + rh[4];
    ans[5] =  m[5]  + rh[5];
    ans[6] =  m[6]  + rh[6];
    ans[7] =  m[7]  + rh[7];
    ans[8] =  m[8]  + rh[8];
    ans[9] =  m[9]  + rh[9];
    ans[10] = m[10] + rh[10];
    ans[11] = m[11] + rh[11];
    ans[12] = m[12] + rh[12];
    ans[13] = m[13] + rh[13];
    ans[14] = m[14] + rh[14];
    ans[15] = m[15] + rh[15];

    return ans;
}


Matrix44 Matrix44::operator-(const Matrix44& rh) const 
{
    Matrix44 ans;
    
    ans[0] =  m[0]  - rh[0];
    ans[1] =  m[1]  - rh[1];
    ans[2] =  m[2]  - rh[2];
    ans[3] =  m[3]  - rh[3];
    ans[4] =  m[4]  - rh[4];
    ans[5] =  m[5]  - rh[5];
    ans[6] =  m[6]  - rh[6];
    ans[7] =  m[7]  - rh[7];
    ans[8] =  m[8]  - rh[8];
    ans[9] =  m[9]  - rh[9];
    ans[10] = m[10] - rh[10];
    ans[11] = m[11] - rh[11];
    ans[12] = m[12] - rh[12];
    ans[13] = m[13] - rh[13];
    ans[14] = m[14] - rh[14];
    ans[15] = m[15] - rh[15];

    return ans;
}


Matrix44 Matrix44::operator-(F32 scalar) const
{
    Matrix44 ans;
    
    ans[0] =  m[0]  - scalar;
    ans[1] =  m[1]  - scalar;
    ans[2] =  m[2]  - scalar;
    ans[3] =  m[3]  - scalar;
    ans[4] =  m[4]  - scalar;
    ans[5] =  m[5]  - scalar;
    ans[6] =  m[6]  - scalar;
    ans[7] =  m[7]  - scalar;
    ans[8] =  m[8]  - scalar;
    ans[9] =  m[9]  - scalar;
    ans[10] = m[10] - scalar;
    ans[11] = m[11] - scalar;
    ans[12] = m[12] - scalar;
    ans[13] = m[13] - scalar;
    ans[14] = m[14] - scalar;
    ans[15] = m[15] - scalar;

    return ans;
}

F32 Matrix44::get(U32 row, U32 col) const
{
    R_ASSERT(row < 4 && col < 4);

    return m[4u * row + col];
}


F32& Matrix44::get(U32 row, U32 col)
{
    R_ASSERT(row < 4 && col < 4);
    
    return m[4u * row + col];
}

void Matrix44::operator+=(const Matrix44& rh)
{
    m[0]  += rh[0];
    m[1]  += rh[1];
    m[2]  += rh[2];
    m[3]  += rh[3];
    m[4]  += rh[4];
    m[5]  += rh[5];
    m[6]  += rh[6];
    m[7]  += rh[7];
    m[8]  += rh[8];
    m[9]  += rh[9];
    m[10] += rh[10];
    m[11] += rh[11];
    m[12] += rh[12];
    m[13] += rh[13];
    m[14] += rh[14];
    m[15] += rh[15];
}


void Matrix44::operator+=(F32 scalar)
{
    m[0]  += scalar;
    m[1]  += scalar;
    m[2]  += scalar;
    m[3]  += scalar;
    m[4]  += scalar;
    m[5]  += scalar;
    m[6]  += scalar;
    m[7]  += scalar;
    m[8]  += scalar;
    m[9]  += scalar;
    m[10] += scalar;
    m[11] += scalar;
    m[12] += scalar;
    m[13] += scalar;
    m[14] += scalar;
    m[15] += scalar;
}


void Matrix44::operator-=(const Matrix44& rh)
{
    m[0]  -= rh[0];
    m[1]  -= rh[1];
    m[2]  -= rh[2];
    m[3]  -= rh[3];
    m[4]  -= rh[4];
    m[5]  -= rh[5];
    m[6]  -= rh[6];
    m[7]  -= rh[7];
    m[8]  -= rh[8];
    m[9]  -= rh[9];
    m[10] -= rh[10];
    m[11] -= rh[11];
    m[12] -= rh[12];
    m[13] -= rh[13];
    m[14] -= rh[14];
    m[15] -= rh[15];
}


void Matrix44::operator-=(F32 scalar)
{
    m[0]  -= scalar;
    m[1]  -= scalar;
    m[2]  -= scalar;
    m[3]  -= scalar;
    m[4]  -= scalar;
    m[5]  -= scalar;
    m[6]  -= scalar;
    m[7]  -= scalar;
    m[8]  -= scalar;
    m[9]  -= scalar;
    m[10] -= scalar;
    m[11] -= scalar;
    m[12] -= scalar;
    m[13] -= scalar;
    m[14] -= scalar;
    m[15] -= scalar;
}


void Matrix44::operator*=(F32 scalar)
{
    m[0]  *= scalar;
    m[1]  *= scalar;
    m[2]  *= scalar;
    m[3]  *= scalar;
    m[4]  *= scalar;
    m[5]  *= scalar;
    m[6]  *= scalar;
    m[7]  *= scalar;
    m[8]  *= scalar;
    m[9]  *= scalar;
    m[10] *= scalar;
    m[11] *= scalar;
    m[12] *= scalar;
    m[13] *= scalar;
    m[14] *= scalar;
    m[15] *= scalar;
}


void Matrix44::operator/=(F32 scalar)
{
    m[0]  /= scalar;
    m[1]  /= scalar;
    m[2]  /= scalar;
    m[3]  /= scalar;
    m[4]  /= scalar;
    m[5]  /= scalar;
    m[6]  /= scalar;
    m[7]  /= scalar;
    m[8]  /= scalar;
    m[9]  /= scalar;
    m[10] /= scalar;
    m[11] /= scalar;
    m[12] /= scalar;
    m[13] /= scalar;
    m[14] /= scalar;
    m[15] /= scalar;
}


void Matrix44::operator*=(const Matrix44& rh)
{
    R_NO_IMPL();
}


Matrix44 perspectiveLH_Aspect(F32 fov, F32 aspect, F32 ne, F32 fa)
{
    Matrix44 persp;
    F32 tanFoV  = tanf(fov * 0.5f);
    F32 yS      = 1.0f / tanFoV;
    F32 xS      = yS / aspect;
    
    persp[0]    = xS;
    persp[5]    = yS;
    persp[10]   = fa / (fa - ne);
    persp[11]   = 1.0f;
    persp[14]   = -ne * fa / (fa - ne);

    return persp;
}


Matrix44 perspectiveRH_Aspect(F32 fov, F32 aspect, F32 ne, F32 fa)
{
    Matrix44 persp;

    F32 tanFoV  = tanf(fov * 0.5f);
    F32 yS      = 1.0f / tanFoV;
    F32 xS      = yS / aspect;

    persp[0]    = xS;
    persp[5]    = yS;
    persp[10]   = fa / (ne - fa);
    persp[11]   = -1.0f;
    persp[14]   = ne * fa / (ne - fa);

    return persp;
}


Matrix44 perspectiveLH(F32 w, F32 h, F32 ne, F32 fa)
{
    Matrix44 persp;

    persp[0]    = 2 * ne / w;
    persp[5]    = 2 * ne / h;
    persp[10]   = fa / (fa - ne);
    persp[11]   = 1.0f;
    persp[14]   = ne * fa / (ne - fa);

    return persp;
}


Matrix44 perspectiveRH(F32 w, F32 h, F32 ne, F32 fa)
{
    Matrix44 persp;

    persp[0]    = 2 * ne / w;
    persp[5]    = 2 * ne / h;
    persp[10]   = fa / (ne - fa);
    persp[11]   = -1.0f;
    persp[14]   = ne * fa / (ne - fa);

    return persp;
}


Float4 operator*(const Matrix44& lh, const Float4& rh)
{
    Float4 res;
    res[0] = lh[0]  * rh[0] + lh[1]  * rh[1] + lh[2]  * rh[2] + lh[3]  * rh[3];
    res[1] = lh[4]  * rh[0] + lh[5]  * rh[1] + lh[6]  * rh[2] + lh[7]  * rh[3];
    res[2] = lh[8]  * rh[0] + lh[9]  * rh[1] + lh[10] * rh[2] + lh[11] * rh[3];
    res[3] = lh[12] * rh[0] + lh[13] * rh[1] + lh[14] * rh[2] + lh[15] * rh[3];
    return res;
}


Matrix44::operator Matrix43 ()
{
    return Matrix43
        (
            m[0],  m[1],  m[2],
            m[4],  m[5],  m[6],
            m[8],  m[9],  m[10],
            m[12], m[13], m[14]
        );
}


Matrix44::operator Matrix43 () const
{
    return Matrix43
        (
            m[0],  m[1],  m[2],
            m[4],  m[5],  m[6],
            m[8],  m[9],  m[10],
            m[12], m[13], m[14]
        );
}


Matrix44 lookAtLH(const Float3& pos, const Float3& target, const Float3& up)
{
    Float3 z = normalize(target - pos);
    Float3 x = normalize(cross(up, z));
    Float3 y = cross(z, x);

    return Matrix44
        (
            x[0],           y[0],           z[0],           0.f,
            x[1],           y[1],           z[1],           0.f,
            x[2],           y[2],           z[2],           0.f,
            -dot(x, pos),   -dot(y, pos),   -dot(z, pos),   1.f
        );
}


Matrix44 lookAtRH(const Float3& pos, const Float3& target, const Float3& up)
{
    Float3 z = normalize(pos - target);
    Float3 x = normalize(cross(up, z));
    Float3 y = cross(z, x);

    return Matrix44
        (
            x[0],           y[0],           z[0],           0.f,
            x[1],           y[1],           z[1],           0.f,
            x[2],           y[2],           z[2],           0.f,
            -dot(x, pos),   -dot(y, pos),   -dot(z, pos),   1.f
        );
}
} // Math
} // Recluse