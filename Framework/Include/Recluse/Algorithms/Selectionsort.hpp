//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

namespace Recluse {

template<typename T, 
         typename Cmp = SimpleCompare<T>>
static void selectionSort(T* pArr, U64 start, U64 sz)
{
    Cmp cmp = { };

    for (U64 i = 0; i < (sz - 1); ++i) 
    {
        U64 desired = i;

        for (U64 j = (i + 1); j < sz; ++j) 
        {
            if (cmp(pArr[desired], pArr[j])) 
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