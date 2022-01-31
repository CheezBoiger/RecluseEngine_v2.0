// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace ECS {


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

// System is the high level provision that oversees all 
// game components to their respect.
template<typename Comp>
class SystemDefinition : public System
{
public:
    virtual         ~SystemDefinition() { } 
    virtual ErrType allocateComponent(Comp** pOut)  = 0;
    virtual ErrType freeComponent(Comp** pIn)       = 0;
};

typedef void* SystemPtr;

} // ECS
} // Recluse