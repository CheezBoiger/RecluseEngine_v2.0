//
#pragma once

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {


struct Ray {
    Float3 o;
    Float3 dir;
};


Float3 rayP(const Ray& ray, F32 t);
} // Recluse