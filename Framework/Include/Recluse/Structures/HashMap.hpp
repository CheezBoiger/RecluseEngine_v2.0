//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"
#include "Recluse/Algorithms/Common.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Structures/RBTree.hpp"

#include <unordered_map>

namespace Recluse {


template<typename Key, typename Value, typename Comparator = CompareLess<Key>, typename _Allocator = MallocAllocator>
class HashMap
{
private:
    struct Pair
    {
        Key     key;
        Value   value;
    };

    struct PairCompare
    {
        Bool operator() (const Pair& lh, const Pair& rh) const 
        {
            return lh.key <= rh.key;
        }
    };


    struct PairEqual
    {
        Bool operator() (const Pair& lh, const Pair& rh) const
        {
            return (lh.key == rh.key) && (lh.value == rh.value);
        }
    };
public:

    typedef Key&                    KeyReference;
    typedef const Key&              ConstantKeyReference;
    typedef Value&                  ValueReference;
    typedef const Value&            ConstantValueReference;

    HashTable();

    void        insert(ConstantKeyReference key, ConstantValueReference value);
    void        remove(ConstantKeyReference key, ConstantValueReference value);

    void        lookup(ConstantKeyReference key);
private:

    SizeT       m_totalSize;
    SizeT       m_numObjects;

    _Allocator  m_allocator;
    Comparator  m_compare;

    // Table with pairs.
    RBTree<Pair, PairCompare, PairEqual>* m_table;
};


// Map container is a helper class to the std unordered map.
// This serves an ADT for easier access.
template<class K, class V, class MapType = std::unordered_map<K, V>>
class MapContainer
{
    typedef V*              Iterator;
    typedef const V*        ConstIterator;
    typedef V&              Reference;
    typedef const V&        ConstReference;
public:
    MapContainer(MapType* keys = nullptr)
        : pMap(keys)
    {
    }

    Bool                        isValid() const { return pMap ? true : false; }

    MapType&   get() { return (*pMap); }
   
    Reference                   operator[](K i) { return (*pMap)[i]; }
    ConstReference              operator[](K i) const { return (*pMap)[i]; }
        
private:
    MapType* pMap;
};
} // Recluse 