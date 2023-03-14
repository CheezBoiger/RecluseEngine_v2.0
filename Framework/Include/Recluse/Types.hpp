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
static R_FORCE_INLINE PtrType pointerSizeBytes()
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
    RecluseResult_NotFound,
    RecluseResult_AlreadyExists,
    RecluseResult_UnknownError,
    RecluseResult_Unexpected
};


template<typename ToCast, typename Class>
static R_FORCE_INLINE ToCast staticCast(Class obj)
{
    return static_cast<ToCast>(obj);
}

// Common Reference counter object. To handle references of a shared object.
class ReferenceCounter
{
public:
    ReferenceCounter()
        : m_count(nullptr)
    {
    }

    ReferenceCounter(const ReferenceCounter& ref) { m_count = ref.m_count; increment(); }
    ReferenceCounter(ReferenceCounter&& ref) noexcept { m_count = ref.m_count; ref.m_count = nullptr; }

    ReferenceCounter& operator=(const ReferenceCounter& ref)
    {
        m_count = ref.m_count;
        increment();
        return (*this);
    }

    ReferenceCounter& operator=(ReferenceCounter&& ref) noexcept
    {
        m_count = ref.m_count;
        ref.m_count = nullptr;
        return (*this);
    }

    virtual ~ReferenceCounter()
    {
    }

    U32 getCount() const { return *m_count;  }
    U32 operator()() const { return getCount(); }

    // Add a reference to the counter. Increments the counter value by one.
    void add() 
    {
        if (!m_count)
            m_count = new U32(0);
        increment(); 
    }

    // Release by decrementing the reference counter on this object. Returns the 
    // result of the decrement. Returns 0 if this reference counter has no more reference, or was already released.
    U32  release() 
    { 
        if (!m_count) return 0;
        decrement();
        U32 count = getCount();
        if (hasNoReferences())
        {
            delete m_count;
            m_count = nullptr;
        }
        return count; 
    }

    Bool hasReferences() const { return m_count ? (getCount() > 0) : false; }
    Bool hasNoReferences() const { return !hasReferences(); }

    //RefCount<T> operator=(const T& data) { return RefCount<T>(); }
protected:
    // Should only call these functions if the object that encapsulates it, needs to remove the counter itself.
    Bool hasCounter() { return m_count; }
    void releaseCounterReference() { m_count = nullptr; }
    void reset() { if (m_count) *m_count = 0; }

private:
    void increment() { ++(*m_count); }
    void decrement() { if ((*m_count) > 0) --(*m_count); }
    U32* m_count;
};


template<typename ClassT>
class DefaultDeleter
{
public:
    void operator()(ClassT* ptr)
    {
        delete ptr;
    }
};


template<typename ClassT>
class DefaultDeleter<ClassT[]>
{
public:
    void operator()(ClassT* ptr)
    {
        delete[] ptr;
    }
};


// Smart pointer system that handles if a pointer is fully released.
// Keeps track of all pointer references.
template<typename ClassT, typename Deleter = DefaultDeleter<ClassT>>
class SmartPtr : public ReferenceCounter
{
public:
    SmartPtr(ClassT* pData = nullptr, Deleter deleter = Deleter())
        : m_pData(pData)
        , m_deleter(deleter)
        , ReferenceCounter()
    {
        if (m_pData)
            add();
    }

    SmartPtr(const SmartPtr& sp)
        : ReferenceCounter(sp)
    {
        m_pData = sp.m_pData;
        m_deleter = sp.m_deleter;
    }

    SmartPtr(SmartPtr&& sp)
        : ReferenceCounter(static_cast<ReferenceCounter&&>(sp))
    {
        m_pData = sp.m_pData;
        m_deleter = sp.m_deleter;
        sp.m_pData = nullptr;
    }

    ~SmartPtr()
    {
        release();
    }
    
    // Release a reference to the smart pointer object.
    // This needs to be called for smart pointers, DO NOT CALL 
    // ReferenceCounter::release() by itself, otherwise it will only decrement
    // the counter, without cleaning up the object (which means you will defer the 
    // deletion of this object until counter is 0, towards the destruction of the smart pointer.)
    U32 release()
    {
        U32 count = ReferenceCounter::release();
        if (count == 0 && m_pData)
        {
            m_deleter(m_pData);
        }
        releaseCounterReference();
        m_pData = nullptr;
        return count;
    }

    ClassT* raw() { return m_pData; }
    const ClassT* raw() const { return m_pData; }
    const ClassT* operator()() const { return m_pData; }
    const ClassT* operator->() const { return m_pData; }
    ClassT* operator->() { return m_pData; }
    ClassT* operator()() { return m_pData; }
    ClassT& operator[](U64 i) { return m_pData[i]; }
    const ClassT& operator[](U64 i) const { return m_pData[i]; }
    Bool operator!() const { return !(m_pData); }
    //Bool operator==(const SmartPtr<ClassT>& rh) const { return (m_pData == rh.m_pData); }
    //Bool operator!=(const SmartPtr<ClassT>& rh) const { return (m_pData != rh.m_pData); }

    operator ClassT* () { return m_pData; }
    operator const ClassT* () const { return m_pData; }

    SmartPtr& operator=(ClassT* ptr)
    {
        m_deleter = Deleter();
        if (m_pData != ptr)
        { 
            reset();
            add();
        }
        m_pData = ptr;
        return (*this);
    }

    SmartPtr& operator=(const SmartPtr& sp)
    {
        ReferenceCounter::operator=(sp);
        m_deleter = sp.m_deleter;
        m_pData = sp.m_pData;
        return (*this);
    }

    SmartPtr& operator=(SmartPtr&& sp) noexcept
    {
        ReferenceCounter::operator=(static_cast<ReferenceCounter&&>(sp));
        m_deleter = sp.m_deleter;
        m_pData = sp.m_pData;
        sp.m_pData = nullptr;
        return (*this);
    }

private:
    ClassT*             m_pData;
    Deleter             m_deleter;
};


// Make a smart pointer object. This might seem redundant though...
template<typename ClassT, typename Deleter = DefaultDeleter<ClassT>>
SmartPtr<ClassT, Deleter> makeSmartPtr(ClassT* pData, Deleter deleter = Deleter())
{
    return static_cast<SmartPtr<ClassT, Deleter>&&>(SmartPtr<ClassT, Deleter>(pData, deleter));
}


// Simpler reference object. This simply holds the object and tracks the number of references, but will not release it
// if the counter hits 0!
template<typename ClassT>
class ReferenceObject : public ReferenceCounter
{
public:
    ReferenceObject()
    {

    }

    ReferenceObject(const ClassT dat)
        : m_dat(dat) { add(); }

    ~ReferenceObject()
    {
    }

    ReferenceObject(const ReferenceObject& obj)
        : ReferenceCounter(obj)
    {
        m_dat = obj.m_dat;
        add();
    }

    ReferenceObject(ReferenceObject&& obj)
        : ReferenceCounter(obj)
    {
        m_dat = obj.m_dat;
    }

    ReferenceObject& operator=(const ReferenceObject& obj)
    {
        ReferenceCounter::operator=(obj);
        m_dat = obj.m_dat;
        return (*this);
    }

    ReferenceObject& operator=(ReferenceObject&& obj)
    {
        ReferenceCounter::operator=(obj);
        m_dat = obj.m_dat;
        return (*this);
    }

    const ClassT& operator()() const { return m_dat; }
    ClassT& operator()() { return m_dat; }

private:
    ClassT m_dat;
};


template<typename ClassT>
ReferenceObject<ClassT> makeReference(const ClassT data)
{
    return ReferenceObject<ClassT>(data);
}


class ICastableObject
{
public:
    template<typename Type>
    R_FORCE_INLINE Type* castTo()
    {
        return static_cast<Type*>(this);
    }

    template<typename Type>
    R_FORCE_INLINE const Type* castTo() const
    {
        return static_cast<const Type*>(this);
    }
};


R_FORCE_INLINE U64 makeBitset64(U64 offset, U64 size, U64 value)
{
    return ((value & ~(0xFFFFFFFFFFFFFFFF << size)) << offset);
}

R_FORCE_INLINE U32 makeBitset32(U32 offset, U32 size, U32 value)
{
    return ((value & ~(0xFFFFFFFF << size)) << offset);
}
} // Recluse
#endif // RECLUSE_TYPES_HPP
