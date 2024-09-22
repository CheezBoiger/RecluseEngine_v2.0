//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector2.hpp"
#include "Recluse/Math/Ray.hpp"

#include "RecluseFramework_exports.hpp"

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
RecluseFramework_PUBLIC_API Bool contains(const Bounds2d& container, const Bounds2d& bounds);

// Check if our bounding boxes intersect.
RecluseFramework_PUBLIC_API Bool intersects(const Bounds2d& a, const Bounds2d& b);
RecluseFramework_PUBLIC_API Bool intersects(const Bounds2d& a, const Ray2d& ray);
RecluseFramework_PUBLIC_API Bool intersects(const Bounds2d& a, const BoundsCircle& circle);
RecluseFramework_PUBLIC_API Bool intersects(const BoundsCircle& a, const BoundsCircle& b);
} // Math
} // Recluse