// 
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


template <typename T>
static void swap(T& lh, T& rh) 
{
    T temp = lh;
    lh = rh;
    rh = temp;
}


template <typename T>
class GenericCompare 
{
 public:
    Bool operator() (const T& lh, const T& rh) const { return lh < rh; }
};
} // Recluse