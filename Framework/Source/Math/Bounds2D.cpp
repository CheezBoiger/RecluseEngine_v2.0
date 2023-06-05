//

#include "Recluse/Math/Bounds2D.hpp"


namespace Recluse {
namespace Math {


Bool contains(const Bounds2d& container, const Bounds2d& bounds)
{
    return (container.mmin.x <= bounds.mmin.x) && (bounds.mmin.x < container.mmax.x)
        && (container.mmin.y <= bounds.mmin.y) && (bounds.mmin.y < container.mmax.y)
        && (container.mmin.x <= bounds.mmax.x) && (bounds.mmax.x < container.mmax.x)
        && (container.mmin.y <= bounds.mmax.y) && (bounds.mmax.y < container.mmax.y);
}


Bool intersects(const Bounds2d& a, const Bounds2d& b)
{
    Bool x = (a.mmax.x >= b.mmin.x) && (a.mmin.x <= b.mmax.x);
    Bool y = (a.mmax.y >= b.mmin.y) && (a.mmin.y <= b.mmax.y);
    return (x && y);
}
} // Math
} // Recluse