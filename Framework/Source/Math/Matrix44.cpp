//
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {


Matrix44::Matrix44(F32 a00, F32 a01, F32 a02, F32 a03,
                   F32 a10, F32 a11, F32 a12, F32 a13,
                   F32 a20, F32 a21, F32 a22, F32 a23,
                   F32 a30, F32 a31, F32 a32, F32 a33)
{
    d[0]  = a00;
    d[1]  = a01;
    d[2]  = a02;
    d[3]  = a03;
    d[4]  = a10;
    d[5]  = a11;
    d[6]  = a12;
    d[7]  = a13;
    d[8]  = a20;
    d[9]  = a21;
    d[10] = a22;
    d[11] = a23;
    d[12] = a30;
    d[13] = a31;
    d[14] = a32;
    d[15] = a33;
}


Matrix44 rotate(const Matrix44& lh, const Float3& ax, F32 radians)
{
    F32 cosine          = cosf(radians);
    F32 sine            = sinf(radians);
    F32 oneMinusCosine  = 1.0f - cosine;

    Float3 axis         = normalize(ax);
    
    // Solves Rodrigues' Rotation Formula, applied to the final result. 
    Matrix44 rotator = {
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

    ans[3]          = lh[12];
    ans[7]          = lh[13];
    ans[11]         = lh[14];
    ans[12]         = lh[3];
    ans[13]         = lh[7];
    ans[14]         = lh[11];

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
    return lh[0] * (lh[5] * (lh[10] * lh[15] - lh[11] * lh[14]) -
                        lh[6] * (lh[9] * lh[15] - lh[11] * lh[13]) +
                        lh[7] * (lh[9] * lh[14] - lh[10] * lh[13])
                    ) -
          lh[1] * (lh[4] * (lh[10] * lh[15] - lh[11] * lh[14]) -
                        lh[6] * (lh[8] * lh[15] - lh[11] * lh[12]) +
                        lh[7] * (lh[8] * lh[14] - lh[10] * lh[12])
                    ) +
          lh[2] * (lh[4] * (lh[9] * lh[15] - lh[11] * lh[13]) -
                        lh[5] * (lh[8] * lh[15] - lh[11] * lh[12]) +
                        lh[7] * (lh[8] * lh[13] - lh[9] * lh[12])
                    ) -
          lh[3] * (lh[4] * (lh[9] * lh[14] - lh[10] * lh[13]) -
                        lh[5] * (lh[8] * lh[14] - lh[10] * lh[12]) +
                        lh[6] * (lh[8] * lh[13] - lh[9] * lh[12]) );
}

Matrix44 inverse(const Matrix44& lh)
{
    F32 det = determinant(lh);
    if (det == 0.f)
        return identity();
    return adjugate(lh) * (1.f / det);
}


Matrix44 identity()
{
    return Matrix44();
}


Matrix44 Matrix44::operator*(const Matrix44& rh) const
{
    Matrix44 ans;

    ans[0]  = d[0]  * rh[0] + d[1]  * rh[4] + d[2]  * rh[8]  + d[3]  * rh[12];
    ans[1]  = d[0]  * rh[1] + d[1]  * rh[5] + d[2]  * rh[9]  + d[3]  * rh[13];
    ans[2]  = d[0]  * rh[2] + d[1]  * rh[6] + d[2]  * rh[10] + d[3]  * rh[14];
    ans[3]  = d[0]  * rh[3] + d[1]  * rh[7] + d[2]  * rh[11] + d[3]  * rh[15];
    
    ans[4]  = d[4]  * rh[0] + d[5]  * rh[4] + d[6]  * rh[8]  + d[7]  * rh[12];
    ans[5]  = d[4]  * rh[1] + d[5]  * rh[5] + d[6]  * rh[9]  + d[7]  * rh[13];
    ans[6]  = d[4]  * rh[2] + d[5]  * rh[6] + d[6]  * rh[10] + d[7]  * rh[14];
    ans[7]  = d[4]  * rh[3] + d[5]  * rh[7] + d[6]  * rh[11] + d[7]  * rh[15];

    ans[8]  = d[8]  * rh[0] + d[9]  * rh[4] + d[10] * rh[8]  + d[11] * rh[12];
    ans[9]  = d[8]  * rh[1] + d[9]  * rh[5] + d[10] * rh[9]  + d[11] * rh[13];
    ans[10] = d[8]  * rh[2] + d[9]  * rh[6] + d[10] * rh[10] + d[11] * rh[14];
    ans[11] = d[8]  * rh[3] + d[9]  * rh[7] + d[10] * rh[11] + d[11] * rh[15];

    ans[12] = d[12] * rh[0] + d[13] * rh[4] + d[14] * rh[8]  + d[15] * rh[12];
    ans[13] = d[12] * rh[1] + d[13] * rh[5] + d[14] * rh[9]  + d[15] * rh[13];
    ans[14] = d[12] * rh[2] + d[13] * rh[6] + d[14] * rh[10] + d[15] * rh[14];
    ans[15] = d[12] * rh[3] + d[13] * rh[7] + d[14] * rh[11] + d[15] * rh[15];

    return ans;
}


Matrix44 Matrix44::operator*(F32 scalar) const
{
    Matrix44 ans;
 
    ans[0]  = d[0]  * scalar;
    ans[1]  = d[1]  * scalar;
    ans[2]  = d[2]  * scalar;
    ans[3]  = d[3]  * scalar;
    ans[4]  = d[4]  * scalar;
    ans[5]  = d[5]  * scalar;
    ans[6]  = d[6]  * scalar;
    ans[7]  = d[7]  * scalar;
    ans[8]  = d[8]  * scalar;
    ans[9]  = d[9]  * scalar;
    ans[10] = d[10] * scalar;
    ans[11] = d[11] * scalar;
    ans[12] = d[12] * scalar;
    ans[13] = d[13] * scalar;
    ans[14] = d[14] * scalar;
    ans[15] = d[15] * scalar;

    return ans;
}
} // Recluse