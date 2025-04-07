//
#pragma once 

#include "Recluse/Types.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {

typedef U32 Hash32;
typedef U64 Hash64;
typedef U128 Hash128;

// Use a hash implementation to serialize structures.
RecluseFramework_PUBLIC_API Hash128 recluseHash(const void* dat, U64 szBytes);

// Faster hash meant for smaller data, but can be used for larger sizes.
RecluseFramework_PUBLIC_API Hash64 recluseHashFast(const void* dat, U64 szBytes);

RecluseFramework_PUBLIC_API Hash32 recluseHash32(const void* dat, U64 szBytes);

// Quick jenkin's hash for speed and integrity balance.
RecluseFramework_PUBLIC_API Hash32 hashJenkins(const void* dat, U32 szBytes);
} // Recluse