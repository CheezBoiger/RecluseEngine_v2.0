//
#pragma once 

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {

struct Ray;

struct R_EXPORT Bounds3D {
    Float3 mmin;
    Float3 mmax;
};


R_EXPORT B32     intersects(const Ray& ray, const Bounds3D& bounds);
R_EXPORT B32     intersects(const Bounds3D& a, const Bounds3D& b);
R_EXPORT F32     surfaceArea(const Bounds3D& a);
R_EXPORT Float3  center(const Bounds3D& a);
R_EXPORT Float3  extent(const Bounds3D& a);
} // Recluse