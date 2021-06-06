// Recluse v2.0 (c) All rights reserved.
//
#ifndef RECLUSE_TYPES_HPP
#define RECLUSE_TYPES_HPP

#pragma once

#include "Core/Arch.hpp"
#include <string>

namespace Recluse {


typedef unsigned char U8;
typedef char I8;

typedef unsigned short U16;
typedef short I16;

typedef unsigned U32;
typedef signed I32;

typedef unsigned long long U64;
typedef signed long long I64;

typedef float F32;
typedef double F64;

typedef I8 B8;
typedef I32 B32;
typedef I64 B64;

typedef I64 ErrType;

#if defined(RECLUSE_WINDOWS)
#if defined(RECLUSE_64BIT)
typedef U64 PtrType;
typedef U64 SizeT;
#else
typedef U32 PtrType;
typedef U32 SizeT;
#endif
#endif

} // Recluse
#endif // RECLUSE_TYPES_HPP
