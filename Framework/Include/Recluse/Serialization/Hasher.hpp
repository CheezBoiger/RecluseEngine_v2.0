//
#pragma once 

#include "Recluse/Types.hpp"

namespace Recluse {

typedef U64 Hash64;
// Use a hash implementation to serialize structures.
R_PUBLIC_API Hash64 recluseHash(const void* dat, U64 szBytes);

} // Recluse