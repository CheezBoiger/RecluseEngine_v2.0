//
#include "meow_hash_x64_aesni.h"

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"


namespace Recluse {


Hash64 recluseHash(void* dat, U64 szBytes)
{
    meow_u128 meowHash = MeowHash(0, szBytes, dat);
    Hash64 hash = MeowU64From(meowHash, 0);
    return hash;
}
} // Recluse