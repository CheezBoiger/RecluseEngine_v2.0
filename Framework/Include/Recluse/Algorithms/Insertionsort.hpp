//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Algorithms/Common.hpp"

namespace Recluse { 


template<typename T,
         typename Compare = CompareLess<T>>
static void insertionSort(T* pArr, U64 start, U64 sz)
{
    Compare cmp = { };
    
}
} // Recluse