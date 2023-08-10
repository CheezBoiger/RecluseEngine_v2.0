//
#pragma once 

#include "Recluse/Types.hpp"

namespace Recluse {

typedef U32 Hash32;
typedef U64 Hash64;
typedef U128 Hash128;

// Use a hash implementation to serialize structures.
R_PUBLIC_API Hash128 recluseHash(const void* dat, U64 szBytes);

// Faster hash meant for smaller data, but can be used for larger sizes.
R_PUBLIC_API Hash64 recluseHashFast(const void* dat, U64 szBytes);

R_PUBLIC_API Hash32 recluseHash32(const void* dat, U64 szBytes);
} // Recluse