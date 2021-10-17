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
class SimpleCompare 
{
 public:
  Bool operator() const (const T& lh, const T& rh) const { return lh < rh; }
};
} // Recluse