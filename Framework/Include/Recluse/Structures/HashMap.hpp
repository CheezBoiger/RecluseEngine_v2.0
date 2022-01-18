//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"

#include <unordered_map>

namespace Recluse {


template
    <
        typename Key, 
        typename Value, 
        typename Comparator, 
        typename Allocator,
        typename PairStruct
    >
class HashTable
{
public:

    typedef Key&                    KeyReference;
    typedef const Key&              ConstKeyReference;
    typedef Value&                  ValueReference;
    typedef const Value&            ConstValueReference;
    

private:

    SizeT       m_totalSize;
    SizeT       m_numObjects;

    Allocator   m_allocator;
    Comparator  m_compare;

    // Table with pairs.
    PairStruct* m_table;
};


// Map container is a helper class to the std unordered map.
// This serves an ADT for easier access.
template<class K, class V>
class MapContainer
{
    typedef V*              Iterator;
    typedef const V*        ConstIterator;
    typedef V&              Reference;
    typedef const V&        ConstReference;
public:
    MapContainer(std::unordered_map<K, V>* keys = nullptr)
        : pMap(keys)
    {
    }

    Bool                        isValid() const { return pMap ? true : false; }

    std::unordered_map<K, V>&   get() { return (*pMap); }
   
    Reference                   operator[](K i) { return (*pMap)[i]; }
    ConstReference              operator[](K i) const { return (*pMap)[i]; }
        
private:
    std::unordered_map<K, V>* pMap;
};
} // Recluse 