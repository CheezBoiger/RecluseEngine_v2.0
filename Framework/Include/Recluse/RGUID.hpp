//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

struct RGUID {
    union {
        struct {
            U64 major;
            U64 minor;
        };
        struct {
            U32 hash0;
            U32 hash1;
            U32 hash2;
            U32 hash3;
        };
    };
};
} // Recluse