//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Math/Ray.hpp"

namespace Recluse {
namespace Math {

struct BoundsCircle
{
	Float2	point;
	F32		radius;
};

struct Bounds2d
{
	Float2 mmin, mmax;
};

typedef Bounds2d AlignedBox2d;
typedef Bounds2d Aligned2d;

// Check if bounding box is fully contained in the container.
R_PUBLIC_API Bool contains(const Bounds2d& container, const Bounds2d& bounds);

// Check if our bounding boxes intersect.
R_PUBLIC_API Bool intersects(const Bounds2d& a, const Bounds2d& b);
R_PUBLIC_API Bool intersects(const Bounds2d& a, const Ray& ray);
R_PUBLIC_API Bool intersects(const Bounds2d& a, const BoundsCircle& circle);
R_PUBLIC_API Bool intersects(const BoundsCircle& a, const BoundsCircle& b);
} // Math
} // Recluse