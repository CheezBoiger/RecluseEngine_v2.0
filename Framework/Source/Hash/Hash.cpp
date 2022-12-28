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


Hash64 recluseHash(void* dat, U64 szBytes)
{
    meow_u128 meowHash = MeowHash(MeowDefaultSeed, szBytes, dat);
    Hash64 hash = MeowU64From(meowHash, 0);
    return hash;
}
} // Recluse