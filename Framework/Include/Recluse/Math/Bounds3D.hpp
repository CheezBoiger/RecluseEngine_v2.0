//
#pragma once 

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {
namespace Math {
struct Ray;
struct Plane;

// Bounding sphere structure.
struct R_PUBLIC_API BoundsSphere
{
    Float3  point;
    F32     radius;
};


// Bounding box structure.
struct R_PUBLIC_API Bounds3d 
{
    Float3 mmin;
    Float3 mmax;
};


typedef Bounds3d Aligned3d;
typedef Bounds3d AlignedBox3d;


R_PUBLIC_API Bool    intersects(const Ray& ray, const Bounds3d& bounds, F32& t);
R_PUBLIC_API Bool    intersects(const Bounds3d& a, const Bounds3d& b);
R_PUBLIC_API Bool    intersects(const Bounds3d& a, const BoundsSphere& sp);
R_PUBLIC_API Bool    intersects(const BoundsSphere& sp, const Bounds3d& b);
R_PUBLIC_API Bool    intersects(const BoundsSphere& sp0, const BoundsSphere& sp1);
R_PUBLIC_API Bool    intersects(const Plane& plane, const Bounds3d& bounds);
R_PUBLIC_API Bool    contains(const Bounds3d& container, const Bounds3d& bounds);
R_PUBLIC_API F32     surfaceArea(const Bounds3d& a);
R_PUBLIC_API Float3  center(const Bounds3d& a);
R_PUBLIC_API Float3  extent(const Bounds3d& a);
} // Math
} // Recluse