//
#pragma once 

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

namespace Recluse {


template<typename T, 
         class Compare = CompareLess<T>>
static void bubbleSort(T* pArr, U64 start, U64 sz)
{
    Compare compare = { };
    Bool swapped = false;

    while (swapped) 
    {
        swapped = false;

        for (U64 i = 0; i < sz; ++i) 
        {
            if (compare(pArr[i - 1], pArr[i])) 
            {
                swap(pArr[i - 1], pArr[i]);
                swapped = true;
            }
        }

        // We know that our last element is going to be sorted with largest.
        sz = sz - 1;
    }
}
} // Recluse