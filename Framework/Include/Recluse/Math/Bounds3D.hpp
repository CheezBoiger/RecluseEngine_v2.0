//
#pragma once 

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {

struct Ray;
struct Plane;

struct R_PUBLIC_API Bounds3d 
{
    Float3 mmin;
    Float3 mmax;
};


R_PUBLIC_API Bool    intersects(const Ray& ray, const Bounds3d& bounds, F32& t);
R_PUBLIC_API Bool    intersects(const Bounds3d& a, const Bounds3d& b);
R_PUBLIC_API Bool    intersects(const Plane& plane, const Bounds3d& bounds);
R_PUBLIC_API F32     surfaceArea(const Bounds3d& a);
R_PUBLIC_API Float3  center(const Bounds3d& a);
R_PUBLIC_API Float3  extent(const Bounds3d& a);
} // Recluse