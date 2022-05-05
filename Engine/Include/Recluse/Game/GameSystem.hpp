// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace ECS {


//! System is the high level provision that oversees all
//! game components to their respect.
class System
{
public:

    virtual ~System() { }

    void setPriority(U32 priority) { m_priority = priority; }
    U32 getPriority() const { return m_priority; }


    virtual ErrType onInitialize()                  = 0;
    virtual ErrType onCleanUp()                     = 0;
    virtual void    updateComponents(F32 deltaTime) = 0;
    virtual ErrType clearAll()                      = 0;

private:
    U32 m_priority;
};


//! SystemDefinition is the required definition of the given system, which is to 
//! define how to allocate, free, and update all components the application interacts 
//! with. Do not inherit directly from System, instead inherit from this!
template<typename Comp>
class SystemDefinition : public System
{
public:
    virtual         ~SystemDefinition() { } 
    virtual ErrType allocateComponent(Comp** pOut)  = 0;
    virtual ErrType freeComponent(Comp** pIn)       = 0;
};


class SystemComparer
{
public:
    Bool operator()(const System& lh, const System& rh) const 
    {
        return lh.getPriority() < rh.getPriority();
    }
};


class SystemPointerComparer
{
public:
    Bool operator()(const System* lh, const System* rh) const
    {
        return lh->getPriority() < rh->getPriority();
    }
};

typedef void* SystemPtr;

} // ECS
} // Recluse