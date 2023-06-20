//
#include "Recluse/Math/Ray.hpp"


namespace Recluse {
namespace Math {



Float3 rayPoint(const Ray& ray, F32 t)
{
    return (ray.o + ray.dir * t);
}
} // Math
} // Recluse