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
void merge(Data* B, I64 begin, I64 middle, I64 end, Data* A, const Compare& comp)
{
    I64 i = begin;
    I64 j = middle;

    // Do the actual merging.
    for (I64 k = begin; k < end; ++k)
    {
        if (i < middle && (j >= end || comp(A[i], A[j])))
        {
            B[k] = A[i];
            i = i + 1;
        }
        else
        {
            B[k] = A[j];
            j = j + 1;
        }
    }
}


template<typename Data, typename Compare = CompareLess<Data>>
void mergeHelper(Data* B, I64 start, I64 end, Data* A, const Compare& comp = Compare())
{
    if (end - start <= 1) return;
    U64 middle = (end + start) / 2;
    mergeHelper(A, start, middle, B, comp);
    mergeHelper(A, middle, end, B, comp);
    merge(B, start, middle, end, A, comp); 
}
} // MergeSortInternal


// Performs an O(n log (n)) merge sort of the following array data structure. This does not 
// handle linked list type data structures. 
// 
// Keep in mind that the space required is O(n), meaning it may require the same amount of 
// space as the unsorted array itself.
template<typename Data, template <typename T> typename Compare>
void mergeSort(Data* pDataArr, I64 count, Data* aux = nullptr)
{
    Bool isInternalMalloc = false;
    Compare<Data> comp = { };
    if (!aux)
    {
        aux = new Data[count];
        isInternalMalloc = true;
    }

    MergeSortInternal::copyArray(aux, 0, count, pDataArr);
    MergeSortInternal::mergeHelper(pDataArr, 0, count, aux, comp);

    if (isInternalMalloc)
    {
        delete aux;
    }
}
} // Recluse