//
#pragma once

#include "Recluse/Arch.hpp"

#include <memory>

#define R_ALLOC_MASK(m, a)      (((m) + ((a)-1)) & ~((a)-1))
#define R_1KB                   (1024ULL)
#define R_1MB                   (1024ULL * 1024ULL)
#define R_1GB                   (1024ULL * 1024ULL * 1024ULL)

#define R_R_SHIFT(m, s)         ((m) >> (s))
#define R_L_SHIFT(m, s)         ((m) << (s))


namespace Recluse {

template<class T, class S>
T* rDynamicCast(S* obj)
{
    return static_cast<T*>(obj);
}
} // Recluse