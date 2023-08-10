//

#if !defined(R_USE_XXHASH)
    #include "meow_hash_x64_aesni.h"
    #define USE_XXHASH 0
#else
    #define XXH_IMPLEMENTATION 1
    #define XXH_STATIC_LINKING_ONLY 1
    #include "xxHash/xxh3.h"
    #define USE_XXHASH 1
#endif

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {


Hash128 recluseHash(const void* dat, U64 szBytes)
{
    Hash128 hash;
#if USE_XXHASH
    static XXH64_hash_t hashSeed = 0;
    XXH128_hash_t hash128 = XXH128(dat, szBytes, hashSeed); 
    hash = *(U128*)&hash128;
#else
    meow_u128 meowHash = MeowHash(MeowDefaultSeed, szBytes, const_cast<void*>(dat));
    hash = *(U128*)&meowHash;
#endif
    return hash;
}


Hash64 recluseHashFast(const void* dat, U64 szBytes)
{
    Hash64 hash;
#if USE_XXHASH
    static XXH64_hash_t hashSeed = 0;
    const XXH64_hash_t hash64 = XXH64(dat, szBytes, hashSeed);
    hash = hash64;
#else
    const meow_u128 hash128 = MeowHash(MeowDefaultSeed, szBytes, const_cast<void*>(dat));
    const meow_u64 hash64 = MeowU64From(hash128, 0);
    hash = hash64;
#endif
    return hash;
}


Hash32 recluseHash32(const void* dst, U64 szBytes)
{
    Hash32 hash;
#if USE_XXHASH
    static XXH32_hash_t hashseed = 0;
    const XXH32_hash_t hash32 = XXH32(dst, szBytes, hashseed);
    hash = hash32;
#else
    const meow_u128 hash128 = MeowHash(MeowDefaultSeed, szBytes, const_cast<void*>(dst));
    hash = MeowU32From(hash128, 0);
#endif
    return hash;
}
} // Recluse