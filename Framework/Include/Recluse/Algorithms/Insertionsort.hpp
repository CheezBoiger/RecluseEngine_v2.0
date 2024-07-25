//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

namespace Recluse { 


template<typename Data>
void swap(Data* pData, U64 a, U64 b)
{
    if (a == b) return;
    std::swap(pData[a], pData[b]);
}

// Quick insertion sort algorithm, O(n^2)
// It is relatively slow with bigger systems, but should be useful for smaller datasets.
template<typename T,
         typename Compare = CompareLess<T>>
static void insertionSort(T* pArr, U64 start, U64 sz, const Compare& cmp)
{
    Compare cmp = { };
    for (U64 i = start; i < sz; ++i)
    {
        U64 j = i;
        while ( j > 0 && cmp(pArr[j - 1], pArr[j]))
        {
            swap(pArr, j - 1, j);
            j = j - 1;
        }
    }
}
} // Recluse