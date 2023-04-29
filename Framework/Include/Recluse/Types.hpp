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
typedef unsigned int        Uint;

typedef unsigned long long  U64;
typedef signed long long    I64;

typedef float               F32;
typedef double              F64;

typedef struct
{
    U64 m0;
    U64 m1;
} U128;

// Separate boolean datatypes.
typedef U8                  B8;
typedef U32                 B32;
typedef U64                 B64;
typedef bool                Bool;
typedef B32                 Bool32;

// Error type to use for error checking.
typedef U32                 ResultCode;

#if defined(RECLUSE_WINDOWS)
#if defined(RECLUSE_64BIT)
typedef U64 UPtr;
typedef U64 SizeT;
#else
typedef U32 UPtr;
typedef U32 SizeT;
#endif
#endif

// Check the OS architecture bitness size. 
// This is the size of the supported instruction set.
// that works with the certain address size.
#if defined(RECLUSE_64BIT)
#define ARCH_PTR_SZ_BYTES 8
#else
#define ARCH_PTR_SZ_BYTES 4
#endif

#define R_STRINGIFY(s) (#s)

// Function representation of defining the recluse engine pointer size.
// This is the size of the stored address, based on the OS architecture.
static R_FORCE_INLINE UPtr pointerSizeBytes()
{
    return ARCH_PTR_SZ_BYTES;
}

enum RecluseResult 
{
    RecluseResult_Ok,
    RecluseResult_Failed = -999,
    RecluseResult_UnknownError,
    RecluseResult_Unexpected,
    RecluseResult_InvalidArgs,
    RecluseResult_NullPtrExcept,
    RecluseResult_CorruptMemory,
    RecluseResult_NoImpl,
    RecluseResult_Timeout,
    RecluseResult_NeedsUpdate,
    RecluseResult_OutOfMemory,
    RecluseResult_NotFound,
    RecluseResult_AlreadyExists,
    RecluseResult_OutOfBounds
};
} // Recluse
#endif // RECLUSE_TYPES_HPP
