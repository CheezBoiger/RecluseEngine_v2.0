//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

namespace Recluse {

template<typename T, 
         class Compare = CompareLess<T>>
static void selectionSort(T* pArr, U64 start, U64 sz)
{
    Compare compare = { };

    for (U64 i = 0; i < (sz - 1); ++i) 
    {
        U64 desired = i;

        for (U64 j = (i + 1); j < sz; ++j) 
        {
            if (compare(pArr[desired], pArr[j])) 
            {
                desired = j;
            }
        }

        if (desired != i) 
        {
            swap(pArr[i], pArr[desired]);
        }
    }
}
} // Recluse