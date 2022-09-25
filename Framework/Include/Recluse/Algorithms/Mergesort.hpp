//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Algorithms/Common.hpp"

namespace Recluse {
namespace MergeSortInternal { // Internal helpers to run merge sort.


template<typename Data>
void copyArray(Data* pDataDst, U64 begin, U64 end, Data* pDataSrc)
{
    for (U64 i = begin; i < end; ++i)
        pDataDst[i] = pDataSrc[i];
}


template<typename Data, typename Compare = CompareLess<Data>>
void bottomUpMerge(Data* pDataA, U64 right, U64 left, U64 end, Data* aux)
{
    Compare comp = { };
    U64 i = left;
    U64 j = right;

    // Do the actual merging.
    for (U64 k = left; k < end; ++k)
    {
        if (i < right && (j >= end || comp(pDataA[i], pDataA[j])))
        {
            aux[k] = pDataA[i];
            i = i + 1;
        }
        else
        {
            aux[k] = pDataA[j];
            j = j + 1;
        }
    }
}

template<typename Data, typename Compare = CompareLess<Data>>
void mergeHelper(Data* pDataA, U64 start, U64 end, Data* aux)
{

}
} // MergeSortInternal

template<typename Data, typename Compare = CompareLess<Data>>
Data* mergeSort(Data* pDataArr, U64 count, Data* aux = nullptr)
{
    Bool isInternalMalloc = false;
    
    if (!aux)
    {
        aux = new Data[count];
        isInternalMalloc = true;
    }

    MergeSortInternal::copyArray(pDataArr, 0, count, aux);
    MergeSortInternal::mergeHelper(pDataArr, 0, count, aux);

    if (isInternalMalloc)
    {
        delete aux;
    }
}
} // Recluse