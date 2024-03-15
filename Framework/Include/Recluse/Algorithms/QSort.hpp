//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"

#include "Recluse/Algorithms/Common.hpp"
#include "Recluse/Utility.hpp"
#include <algorithm>

namespace Recluse {
namespace QuickSortInternal {


template<typename Data>
void swap(Data* pData, I64 a, I64 b)
{
    if (a == b) return;
    std::swap(pData[a], pData[b]);
}


template<typename Data, typename Compare = CompareLess<Data>>
I32 partition(Data* pData, I64 lo, I64 hi, const Compare& comp)
{
    const Data& pivot = pData[hi];

    I64 i = lo - 1;
    for (I64 j = lo; j < hi; ++j)
    {
        if (comp(pData[j], pivot))
        {
            i += 1;
            swap(pData, i, j);
        }
    }
    i += 1;
    swap(pData, i, hi);
    return i;
}


template<typename Data, typename Compare = CompareLess<Data>>
void quickSortHelper(Data* pData, I64 lo, I64 hi, const Compare& comp)
{
    if (lo >= hi || lo < 0)
        return;
    I64 p = QuickSortInternal::partition(pData, lo, hi, comp);
    quickSortHelper(pData, lo, p - 1, comp);
    quickSortHelper(pData, p + 1, hi, comp);
}
} // QuickSortInternal


// Performs average O(n log (n)) quick sort of the following array data structure. Does not 
// handle linked list data structures. Takes a pivot, and sorts based on that.
//
// Worst case O(n^2) if the array is completely unsorted.  
template<typename Data, template <typename T> typename Compare>
void quickSort(Data* data, I64 lo, I64 hi)
{
    if (lo >= hi || lo < 0)
        return;
    Compare<Data> comp = { };
    QuickSortInternal::quickSortHelper(data, lo, hi, comp);
}
} // Recluse