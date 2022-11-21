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

// Function representation of defining the recluse engine pointer size.
static R_FORCE_INLINE PtrType pointerSize()
{
    return ARCH_PTR_SZ_BYTES;
}

enum RecluseResult 
{
    RecluseResult_Ok,
    RecluseResult_Failed = -999,
    RecluseResult_InvalidArgs,
    RecluseResult_NullPtrExcept,
    RecluseResult_CorruptMemory,
    RecluseResult_NoImpl,
    RecluseResult_Timeout,
    RecluseResult_NeedsUpdate,
    RecluseResult_OutOfMemory,
    RecluseResult_NotFound
};


template<typename ToCast, typename Class>
static R_FORCE_INLINE ToCast staticCast(Class obj)
{
    return static_cast<ToCast>(obj);
}

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

    void addRef() { increment(); }

    // Release by decrementing the reference counter on this object. Returns the 
    // result of the decrement.
    U32  release() { decrement(); return m_count; }

    Bool hasRefs() const { return (getCount() > 0); }
    Bool hasNoRefs() const { return !hasRefs(); }

    operator T () { return getData(); }
    //RefCount<T> operator=(const T& data) { return RefCount<T>(); }

private:
    void increment() { ++m_count; }
    void decrement() { if (m_count > 0) --m_count; }

    T m_dat;
    U32 m_count;
};


template<typename Class>
class SmartPtr
{
public:
    SmartPtr(Class* pData = nullptr)
        : pData(pData)
    {
    }

    ~SmartPtr()
    {
        if (pData) delete pData;
        pData = nullptr;
    }

    Class* raw() { return pData; }

    SmartPtr& operator=(Class* ptr)
    {
        pData = ptr;
    }

private:
    Class* pData;
};


class ICastableObject
{
public:
    template<typename Type>
    R_FORCE_INLINE Type* castTo()
    {
        return static_cast<Type*>(this);
    }
};
} // Recluse
#endif // RECLUSE_TYPES_HPP
