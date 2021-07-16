//
#pragma once 

#include "Recluse/Types.hpp"

namespace Recluse {


struct Hash128 {
    U64 hash0;
    U64 hash1;
};


// Use a hash implementation to serialize structures.
Hash128 recluseHash128(void* dat, U64 szBytes);


class Hash {
public:
    
};
} // Recluse