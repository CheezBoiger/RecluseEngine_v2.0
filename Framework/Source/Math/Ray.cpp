//
#include "Recluse/Math/Ray.hpp"


namespace Recluse {
namespace Math {



Float3 rayPoint(const Ray3d& ray, F32 t)
{
    return (ray.o + ray.dir * t);
}
} // Math
} // Recluse