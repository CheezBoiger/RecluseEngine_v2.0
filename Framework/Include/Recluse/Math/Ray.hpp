//
#pragma once

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {
namespace Math { 

struct R_PUBLIC_API Ray 
{
    Float3 o;
    Float3 dir;

    Ray(const Float3& origin = Float3(), const Float3& direction = Float3())
        : o(origin)
        , dir(direction) 
    { }
};


R_PUBLIC_API Float3 rayPoint(const Ray& ray, F32 t);
} // Math
} // Recluse