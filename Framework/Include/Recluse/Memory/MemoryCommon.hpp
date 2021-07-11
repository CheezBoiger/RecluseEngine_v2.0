//
#pragma once

#include "Recluse/Arch.hpp"

#define R_ALLOC_MASK(m, a)      (((m) + ((a)-1)) & ~((a)-1))
#define R_1KB                   (1024ULL)
#define R_1MB                   (1024ULL * 1024ULL)
#define R_1GB                   (1024ULL * 1024ULL * 1024ULL)

#define R_R_SHIFT(m, s)         ((m) >> (s))
#define R_L_SHIFT(m, s)         ((m) << (s))