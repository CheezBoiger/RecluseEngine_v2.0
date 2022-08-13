//
#pragma once

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/MathIntrinsics.hpp"
#include "Recluse/Math/Matrix44.hpp"

namespace Recluse {


class Matrix34 
{
public:
    union 
    {
        F32 m[12];
        struct 
        {
            __m128 row0;
            __m128 row1;
            __m128 row2;
        };
    };



    operator Matrix44 () 
    {
        return Matrix44(

        );
    }
};
} // Recluse