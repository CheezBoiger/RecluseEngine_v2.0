//
#pragma once 

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {

struct Ray;

struct R_PUBLIC_API Bounds3D {
    Float3 mmin;
    Float3 mmax;
};


R_PUBLIC_API B32     intersects(const Ray& ray, const Bounds3D& bounds);
R_PUBLIC_API B32     intersects(const Bounds3D& a, const Bounds3D& b);
R_PUBLIC_API F32     surfaceArea(const Bounds3D& a);
R_PUBLIC_API Float3  center(const Bounds3D& a);
R_PUBLIC_API Float3  extent(const Bounds3D& a);
} // Recluse