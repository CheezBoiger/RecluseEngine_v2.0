//
#include "Recluse/Math/Matrix43.hpp"
#include "Recluse/Math/Matrix44.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {
namespace Math {

Matrix43::Matrix43
    (
        F32 a00, F32 a01, F32 a02,
        F32 a10, F32 a11, F32 a12,
        F32 a20, F32 a21, F32 a22,
        F32 a30, F32 a31, F32 a32
    )
{
    m[0] = a00; m[1]  = a01; m[2]  = a02;
    m[3] = a10; m[4]  = a11; m[5]  = a12;
    m[6] = a20; m[7]  = a21; m[8]  = a22;
    m[9] = a30; m[10] = a31; m[11] = a32;
}


Matrix43::operator Matrix44 ()
{
    return Matrix44
        (
            m[0], m[1],  m[2],  0.0f,
            m[3], m[4],  m[5],  0.0f,
            m[6], m[7],  m[8],  0.0f,
            m[9], m[10], m[11], 1.0f
        );
}


Matrix43::operator Matrix44 () const
{
    return Matrix44
        (
            m[0], m[1],  m[2],  0.0f,
            m[3], m[4],  m[5],  0.0f,
            m[6], m[7],  m[8],  0.0f,
            m[9], m[10], m[11], 1.0f
        );
}


F32 Matrix43::get(U32 row, U32 col) const
{
    R_ASSERT(row < 4 && col < 3);
    return m[3u * row + col];
}


F32& Matrix43::get(U32 row, U32 col)
{
    R_ASSERT(row < 4 && col < 3);
    return m[3u * row + col];
}
} // Math
} // Recluse