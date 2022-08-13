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

// Error type to use for error checking.
typedef U32                 ErrType;

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

enum RResult 
{
    R_RESULT_OK,
    R_RESULT_FAILED = -999,
    R_RESULT_INVALID_ARGS,
    R_RESULT_NULL_PTR_EXCEPT,
    R_RESULT_CORRUPT_MEMORY,
    R_RESULT_NO_IMPL,
    R_RESULT_TIMEOUT,
    R_RESULT_NEEDS_UPDATE,
    R_RESULT_OUT_OF_MEMORY,
    R_RESULT_NOT_FOUND
};

// Common Reference counter object. To handle references of a shared object.
// 
template<typename T>
class RefCount
{
public:
    RefCount()
        : m_count(0)
        , m_dat(T()) 
    {
        increment();
    }

    RefCount(const T& data) { m_count = 0; m_dat = data; }

    RefCount(const RefCount& ref)
    {
        m_count = ref.getCount();
        m_dat = ref.m_dat;
        increment();
    }

    ~RefCount()
    {
    }

    U32 getCount() const { return m_count;  }

    T& getData() { return m_dat; }

    T& operator()() { return getData(); }

    void add() { increment(); }
    void release() { decrement(); }

    operator T () { return getData(); }
    //RefCount<T> operator=(const T& data) { return RefCount<T>(); }

private:
    void increment() { ++m_count; }
    void decrement() { --m_count; }

    T m_dat;
    U32 m_count;
};

} // Recluse
#endif // RECLUSE_TYPES_HPP
