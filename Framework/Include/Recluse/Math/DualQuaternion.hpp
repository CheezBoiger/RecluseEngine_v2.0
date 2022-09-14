//
#pragma once

#include "Recluse/Math/Quaternion.hpp"

namespace Recluse {


class DualQuaternion
{
    Quaternion part;
    Quaternion dual;
};
} // Recluse