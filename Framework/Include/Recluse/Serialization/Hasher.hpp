//
#pragma once 

#include "Recluse/Types.hpp"

namespace Recluse {

typedef U64 Hash64;
// Use a hash implementation to serialize structures.
Hash64 recluseHash(void* dat, U64 szBytes);

} // Recluse