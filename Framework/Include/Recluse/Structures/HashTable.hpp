//
#include "Recluse/Types.hpp"

namespace Recluse {


template<typename _Key, typename _Value, typename _Hash, typename _Comparer, typename _CompareEqual, typename _Allocator>
class HashTable
{
private:

    struct KeyValuePair
    {
        _Key    _key;
        _Value  _value;
    };

public:

    HashTable();

private:
    _Comparer       _comparer;
    _CompareEqual   _equal;
    _Allocator      _allocator;
    _Hash           _hasher;
    KeyValuePair*   _table;
    U32             _entriesCount;
    U32             _totalEntriesCount;
};
} // Recluse