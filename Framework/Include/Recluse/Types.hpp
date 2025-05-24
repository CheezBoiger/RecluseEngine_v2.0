// Recluse v2.0 (c) All rights reserved.
//
#ifndef RECLUSE_TYPES_HPP
#define RECLUSE_TYPES_HPP

#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Math/half.hpp"
#include <string>

namespace Recluse {


typedef unsigned char       U8;
typedef char                I8;
typedef U8                  u8;
typedef I8                  i8;

typedef unsigned short      U16;
typedef short               I16;
typedef U16                 u16;
typedef I16                 i16;

typedef unsigned            U32;
typedef signed              I32;
typedef unsigned int        UInt;
typedef UInt                uint;
typedef U32                 u32;
typedef I32                 i32;

typedef unsigned long long  U64;
typedef signed long long    I64;
typedef U64                 u64;
typedef I64                 i64;

typedef float               F32;
typedef double              F64;
typedef F32                 f32;
typedef F64                 f64;
typedef F32                 fsingle;

typedef half_float::half    Half;
typedef Half                F16;
typedef F16                 f16;
typedef Half                half;

typedef half                lowp;
typedef F32                 mediump;
typedef F64                 highp;

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
typedef B64                 Bool64;
typedef B8                  b8;
typedef B32                 b32;
typedef B64                 b64;

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

typedef SizeT sizet;
typedef UPtr uptr;

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
    RecluseResult_OutOfBounds,
    RecluseResult_InvalidVersion
};
} // Recluse
#endif // RECLUSE_TYPES_HPP
