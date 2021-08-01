//
#pragma once 

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {

struct Ray;

struct Bounds3D {
    Float3 mmin;
    Float3 mmax;
};


B32     intersects(const Ray& ray, const Bounds3D& bounds);
B32     intersects(const Bounds3D& a, const Bounds3D& b);
F32     area(const Bounds3D& a);
Float3  center(const Bounds3D& a);
} // Recluse