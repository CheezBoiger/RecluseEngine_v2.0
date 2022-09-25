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
    virtual ~GenericCompare() { }
    virtual Bool operator() (const T& lh, const T& rh) const = 0;
};


template <typename T>
class CompareLess : public GenericCompare<T>
{
public:
    Bool operator() (const T& lh, const T& rh) const override { return lh < rh; }
};

template <typename T>
class CompareLessEqual : public GenericCompare<T>
{
public:
    Bool operator() (const T& lh, const T& rh) const override { return lh <= rh; }
};


template <typename T>
class CompareGreater : public GenericCompare<T>
{
public:
    Bool operator() (const T& lh, const T& rh) const override { return lh > rh; }
};

template <typename T>
class CompareGreaterEqual : public GenericCompare<T>
{
public:
    Bool operator() (const T& lh, const T& rh) const override { return lh >= rh; }
};
} // Recluse