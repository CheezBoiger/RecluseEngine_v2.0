// Recluse v2.0 (c) All rights reserved.
//
#ifndef RECLUSE_TYPES_HPP
#define RECLUSE_TYPES_HPP

#pragma once

#include "Recluse/Arch.hpp"
#include <string>

namespace Recluse {


typedef unsigned char       U8;
typedef char                I8;

typedef unsigned short      U16;
typedef short               I16;

typedef unsigned            U32;
typedef signed              I32;

typedef unsigned long long  U64;
typedef signed long long    I64;

typedef float               F32;
typedef double              F64;

// Separate boolean datatypes.
typedef U8                  B8;
typedef U32                 B32;
typedef U64                 B64;
typedef bool                Bool;

// Error type to use for error checking.
typedef I32                 ErrType;

#if defined(RECLUSE_WINDOWS)
#if defined(RECLUSE_64BIT)
typedef U64 PtrType;
typedef U64 SizeT;
#else
typedef U32 PtrType;
typedef U32 SizeT;
#endif
#endif

#if defined(RECLUSE_64BIT)
#define ARCH_PTR_SZ_BYTES 8
#else
#define ARCH_PTR_SZ_BYTES 4
#endif

#define R_STRINGIFY(s) (#s)

enum RecResult 
{
    REC_RESULT_OK,
    REC_RESULT_FAILED = -999,
    REC_RESULT_INVALID_ARGS,
    REC_RESULT_NULL_PTR_EXCEPTION,
    REC_RESULT_CORRUPT_MEMORY,
    REC_RESULT_NOT_IMPLEMENTED,
    REC_RESULT_TIMEOUT,
    REC_RESULT_NEEDS_UPDATE,
    REC_RESULT_OUT_OF_MEMORY,
    REC_RESULT_NOT_FOUND
};

} // Recluse
#endif // RECLUSE_TYPES_HPP
