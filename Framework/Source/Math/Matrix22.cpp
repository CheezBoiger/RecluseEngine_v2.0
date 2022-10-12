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
} // Math
} // Recluse