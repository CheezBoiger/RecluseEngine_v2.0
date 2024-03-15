//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

#include <algorithm>

namespace Recluse {

template<typename Data, 
         template<typename T> typename Compare>
static void selectionSort(Data* pArr, U64 start, U64 sz)
{
    Compare<Data> compare = { };

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
            std::swap(pArr[i], pArr[desired]);
        }
    }
}
} // Recluse