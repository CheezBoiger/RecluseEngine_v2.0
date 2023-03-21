//

#if R_USE_XXHASH
#define XXH_IMPLEMENTATION 1
#define XXH_STATIC_LINKING_ONLY 1
#include "xxHash/xxhash.h"
#else
#include "meow_hash_x64_aesni.h"
#endif
#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"


namespace Recluse {


Hash64 recluseHash(const void* dat, U64 szBytes)
{
    Hash64 hash;
#if R_USE_XXHASH
    XXH64_hash_t hashSeed = 0;
    hash = XXH64(dat, szBytes, hashSeed); 
#else
    meow_u128 meowHash = MeowHash(MeowDefaultSeed, szBytes, const_cast<void*>(dat));
    hash = MeowU64From(meowHash, 0);
#endif
    return hash;
}
} // Recluse