//
#include "Recluse/RGUID.hpp"

#include <random>
#include <algorithm>

namespace Recluse {

RGUID generateRGUID(U64 seed)
{
    U64 seedValue = seed;
    if (seed == 0)
    {
        std::random_device dev;
        seedValue = dev();
    }

    std::mt19937 twister(seedValue);

    RGUID nRGUID;
    nRGUID.version.major = std::uniform_int_distribution<U64>()(twister);
    nRGUID.version.minor = std::uniform_int_distribution<U64>()(twister);

    return nRGUID;
}
} // Recluse