//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Vector2.hpp"

namespace Recluse {
namespace Math {
struct Bounds2d
{
	Float2 mmin, mmax;
};


R_PUBLIC_API Bool contains(const Bounds2d& container, const Bounds2d& bounds);
R_PUBLIC_API Bool intersects(const Bounds2d& a, const Bounds2d& b);
} // Math
} // Recluse