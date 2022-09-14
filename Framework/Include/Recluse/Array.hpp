//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {

template<typename T, typename Alloc = DefaultAllocator>
class Array 
{
public:


    void reserve(U64 capacity);

    void    pushBack(T& o);
    T       popBack();

    T&      top() { return m_arr[m_size - 1]; }

    U64 getSize() const { return m_size; 
    U64 getCapacity() const { return m_capacity; }

    T*          getRaw() { return m_arr; }
    const T*    getRaw() const { return m_arr; }
private:
    T*  m_arr;
    U64 m_size;
    U64 m_capacity;
};
} // Recluse 