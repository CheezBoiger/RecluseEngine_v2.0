//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {

template<typename T, typename Alloc = MallocAllocator>
class Array 
{
public:
    
    typedef T               Type;
    typedef T*              TypePtr;
    typedef T&              TypeRef;
    typedef const T&        ConstTypeRef;
    typedef const T*        ConstTypePtr;
    typedef Array&          StructureRef;
    typedef Array*          StructurePtr;
    typedef const Array&    ConstStructureRef;
    typedef const Array*    ConstStructurePtr;

    Array(U64 initCapacity = 0)
        : m_size(0)
        , m_capacity(initCapacity)
        , m_arr(nullptr)
        , m_alloc(Alloc()) 
    {
        
    }

    Array();

    void    reserve(U64 capacity);

    void    pushBack(TypeRef o);
    Type    popBack();
    void    emplaceBack(TypeRef o);

    TypeRef      top() { return m_arr[m_size - 1]; }

    U64 getSize() const { return m_size; }
    U64 getCapacity() const { return m_capacity; }

    TypePtr         getRaw() { return m_arr; }
    ConstTypePtr    getRaw() const { return m_arr; }
private:
    TypePtr     m_arr;
    U64         m_size;
    U64         m_capacity;
    Alloc       m_alloc;
};
} // Recluse 