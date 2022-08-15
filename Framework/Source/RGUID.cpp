//
#include "Recluse/RGUID.hpp"

#include <random>
#include <algorithm>

namespace Recluse {


RGUID generateRGUID(U64 seed)
{
    std::random_device dev;
    std::mt19937 twister(dev());

    RGUID nRGUID;
    nRGUID.version.major = std::uniform_int_distribution<U64>()(twister);
    nRGUID.version.minor = std::uniform_int_distribution<U64>()(twister);

    return nRGUID;
}
} //