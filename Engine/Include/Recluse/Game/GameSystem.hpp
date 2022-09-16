// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace ECS {


//! AbstractSystem is the high level provision that oversees all
//! game components to their respect.
class AbstractSystem
{
public:

    virtual ~AbstractSystem() { }

    void setPriority(U32 priority) { m_priority = priority; }
    U32 getPriority() const { return m_priority; }


    virtual ErrType onInitialize()                  = 0;
    virtual ErrType onCleanUp()                     = 0;
    virtual void    updateComponents(F32 deltaTime) = 0;
    virtual ErrType clearAll()                      = 0;

    // Obtains the total number of allocated components in the 
    // system.
    virtual U32     getTotalComponents() const      = 0;

private:
    U32 m_priority;
};


//! System is the required definition of the given system, which is to 
//! define how to allocate, free, and update all components the application interacts 
//! with. Do not inherit directly from System, instead inherit from this!
template<typename Comp>
class System : public AbstractSystem
{
public:
    virtual         ~System() { } 

    // Allocates a component from the system pool.
    // Returns R_RESULT_OK if the system successfully allocated the component instance.
    ErrType allocateComponent(Comp** pOut)  
    {
        ErrType err = onAllocateComponent(pOut);
        if (err == R_RESULT_OK) m_numberOfComponentsAllocated += 1;
        return err;
    }

    // Frees up a component from the system pool.
    // Returns R_RESULT_OK if the system successfully freed the component instance.
    ErrType freeComponent(Comp** pIn)
    {
        ErrType err = onFreeComponent(pIn);
        if (err == R_RESULT_OK) m_numberOfComponentsAllocated -= 1;
        return err;
    }

    virtual U32     getTotalComponents() const override { return m_numberOfComponentsAllocated; }

protected:

    virtual ErrType onAllocateComponent(Comp** pOut) = 0;
    virtual ErrType onAllocateComponents(Comp*** pOuts, U32 count) = 0;

    virtual ErrType onFreeComponent(Comp** pIn) = 0;
    virtual ErrType onFreeComponents(Comp*** pOuts, U32 count) = 0;

    U32     m_numberOfComponentsAllocated;
};


class SystemComparer
{
public:
    Bool operator()(const AbstractSystem& lh, const AbstractSystem& rh) const 
    {
        return lh.getPriority() < rh.getPriority();
    }
};


class SystemPointerComparer
{
public:
    Bool operator()(const AbstractSystem* lh, const AbstractSystem* rh) const
    {
        return lh->getPriority() < rh->getPriority();
    }
};

typedef void* SystemPtr;

} // ECS
} // Recluse