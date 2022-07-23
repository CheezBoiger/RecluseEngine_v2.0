//
#pragma once

#include "Recluse/Arch.hpp"

#include <memory>

#define R_ALLOC_MASK(m, a)      (((m) + ((a)-1)) & ~((a)-1))
#define R_1KB                   (1024ULL)
#define R_1MB                   (1024ULL * 1024ULL)
#define R_1GB                   (1024ULL * 1024ULL * 1024ULL)

#define R_BYTES(b)              ((b))
#define R_KB(b)                 (R_1KB * (b))
#define R_MB(b)                 (R_1MB * (b))
#define R_GB(b)                 (R_1GB * (b))

#define R_R_SHIFT(m, s)         ((m) >> (s))
#define R_L_SHIFT(m, s)         ((m) << (s))

// Circular shifts. These require values to be [1 < s < bits]
#define R_R_ROT(m, s, bits)     (((m) >> (s)) | ((m) << ((bits) - (s))))
#define R_L_ROT(m, s, bits)     (((m) << (s)) | ((m) >> ((bits) - (s))))


namespace Recluse {

template<class T, class S>
T* rDynamicCast(S* obj)
{
    return static_cast<T*>(obj);
}


// Just a simple call that applies offset to our baseAddr.
static PtrType offsetOf(PtrType baseAddr, PtrType offsetBytes)
{
    return baseAddr + offsetBytes;
}
} // Recluse