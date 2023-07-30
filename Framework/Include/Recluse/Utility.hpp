// Recluse v2.0 (c) All rights reserved.
#ifndef RECLUSE_UTILITY_HPP
#define RECLUSE_UTILITY_HPP 

#pragma once

#include "Recluse/Types.hpp"
#include <map>

namespace Recluse {



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
        if (hasNoReferences())
        {
            delete m_count;
            m_count = nullptr;
        }
    }

    U32 getCount() const { return (m_count ? *m_count : 0);  }
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
        return getCount(); 
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

    ClassT& operator*() { return *m_pData; }
    const ClassT& operator*() const { return *m_pData; }

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
constexpr SmartPtr<ClassT, Deleter> makeSmartPtr(ClassT* pData, Deleter deleter = Deleter())
{   
    SmartPtr<ClassT, Deleter> ptr(pData, deleter);
    return static_cast<SmartPtr<ClassT, Deleter>&&>(ptr);
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
    const ClassT& operator*() const { return m_dat; }
    ClassT& operator*() { return m_dat; }

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


constexpr R_FORCE_INLINE U64 makeBitset64(U64 offset, U64 size, U64 value)
{
    return ((value & ~(0xFFFFFFFFFFFFFFFF << size)) << offset);
}

constexpr R_FORCE_INLINE U32 makeBitset32(U32 offset, U32 size, U32 value)
{
    return ((value & ~(0xFFFFFFFF << size)) << offset);
}


namespace GlobalCommands {
namespace Internal {
    struct DataListener;
    struct DataListener
    {
        void* value;

        R_PUBLIC_API DataListener(const std::string& command, void* globalVariable);
    private:
        void storeData(const std::string& command, DataListener* data);
    };
    R_PUBLIC_API DataListener* obtainData(const std::string& command);
    R_PUBLIC_API Bool setData(const std::string& command, const void* value, size_t sizeBytesToWrite);
    R_PUBLIC_API Bool setDataAsString(const std::string& command, const char* value);
} // Internal


// Obtain the value of the global command.
template<typename Class>
Class obtainValue(const std::string& command)
{
    Internal::DataListener* pData = Internal::obtainData(command);
    return *reinterpret_cast<Class*>(pData->value);
}

namespace {
// Set the value of the global command.
template<typename Class>
Bool setValue(const std::string& command, Class value)
{
    return Internal::setData(command, &value, sizeof(Class));
}

template<>
Bool setValue(const std::string& command, std::string value)
{
    return Internal::setDataAsString(command, value.c_str());
}

template<>
Bool setValue(const std::string& command, const char* value)
{
    return Internal::setDataAsString(command, value);
}
} // anonymous namespace

#define R_DECLARE_GLOBAL_VARIABLE(varName, defaultValue, commandName, dataType) \
    dataType varName = defaultValue; \
    Recluse::GlobalCommands::Internal::DataListener _listener__ ## varName = Recluse::GlobalCommands::Internal::DataListener(commandName, &varName);

#define R_DECLARE_GLOBAL_BOOLEAN(varName, defaultValue, commandName) R_DECLARE_GLOBAL_VARIABLE(varName, defaultValue, commandName, Recluse::Bool)
#define R_DECLARE_GLOBAL_F32(varName, defaultValue, commandName) R_DECLARE_GLOBAL_VARIABLE(varName, defaultValue, commandName, Recluse::F32)
#define R_DELCARE_GLOBAL_I32(varName, defaultValue, commandName) R_DECLARE_GLOBAL_VARIABLE(varName, defaultValue, commandName, Recluse::I32)
#define R_DECLARE_GLOBAL_U32(varName, defaultValue, commandName) R_DECLARE_GLOBAL_VARIABLE(varName, defaultValue, commandName, Recluse::U32)
#define R_DECLARE_GLOBAL_STRING(varName, defaultValue, commandName) R_DECLARE_GLOBAL_VARIABLE(varName, defaultValue, commandName, std::string)
} // GlobalCommands
} // Recluse
#endif // RECLUSE_UTILITY_HPP