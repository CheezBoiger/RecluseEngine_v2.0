// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {
namespace ECS {

// Declaration types.
typedef U64     GameUUID;
typedef Hash64  RGUID;

// Forward declare game object.
class GameObject;

#define R_PUBLIC_DECLARE_GAME_ECS(_class, guuid) \
    public: \
    static Recluse::ECS::RGUID classGUID() { return recluseHash(guuid, sizeof(guuid)); } \
    static const char* className() { return #_class; } \
    virtual Recluse::ECS::RGUID getClassGUID() const override { return classGUID(); } \
    virtual const char* getClassName() const override { return className(); }


//! AbstractSystem is the high level provision that oversees all
//! game components to their respect.
class R_PUBLIC_API AbstractSystem
{
public:

    virtual ~AbstractSystem() { }

    void setPriority(U32 priority) { m_priority = priority; }
    U32 getPriority() const { return m_priority; }

    void    updateComponents(F32 deltaTime) { onUpdateComponents(deltaTime); }
    void   clearAll()                      { onClearAll(); }

    // Obtains the total number of allocated components in the 
    // system.
    virtual U32     getTotalComponents() const      = 0;

    ErrType         initialize()
    {
        return onInitialize();
    }

private:
    virtual ErrType onInitialize()                  { return R_RESULT_NO_IMPL; }
    virtual ErrType onCleanUp()                     { return R_RESULT_NO_IMPL; }
    virtual void onClearAll()                       { }
    virtual void onUpdateComponents(F32 deltaTime)  { }

    U32 m_priority;
};

// Required declare for the game system to be used. 
// Use the constructor you feel is important.
#define R_DECLARE_GAME_SYSTEM(_constructor, _component) \
    public: \
    static ECS::System<_component>* create() { \
        return new _constructor; \
    }

//! System is the required definition of the given system, which is to 
//! define how to allocate, free, and update all components the application interacts 
//! with. Do not inherit directly from System, instead inherit from this!
template<typename Comp>
class R_PUBLIC_API System : public AbstractSystem
{
public:
    virtual         ~System() { } 

    // Allocates a component from the system pool.
    // Returns R_RESULT_OK if the system successfully allocated the component instance.
    ErrType allocateComponent(Comp** pOut, GameObject* pOwner)  
    {
        ErrType err = onAllocateComponent(pOut);
        if (err == R_RESULT_OK) 
        {
            m_numberOfComponentsAllocated += 1;
            (*pOut)->setOwner(pOwner);
        }
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