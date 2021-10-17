//
#pragma once

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {


struct R_PUBLIC_API Ray {
    Float3 o;
    Float3 dir;
};


R_PUBLIC_API Float3 rayP(const Ray& ray, F32 t);
} // Recluse