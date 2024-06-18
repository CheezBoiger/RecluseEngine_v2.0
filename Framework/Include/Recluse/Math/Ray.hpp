//
#pragma once

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {
namespace Math { 

// 2D ray math.
struct R_PUBLIC_API Ray2d
{
    Float2 o;
    Float2 dir;
    Ray2d(const Float2& origin = Float2(), const Float2& direction = Float2())
        : o(origin)
        , dir(direction) 
    { }
};

// 3D ray math.
struct R_PUBLIC_API Ray3d 
{
    Float3 o;
    Float3 dir;

    Ray3d(const Float3& origin = Float3(), const Float3& direction = Float3())
        : o(origin)
        , dir(direction) 
    { }
};


R_PUBLIC_API Float3 rayPoint(const Ray3d& ray, F32 t);
} // Math
} // Recluse