// 
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Time.hpp"

namespace Recluse {
namespace ECS {

// Declaration types.
typedef Hash64 GameUUID;

// Forward declare game object.
class GameEntity;

#define R_PUBLIC_DECLARE_GAME_ECS(_class) \
    public: \
    static Recluse::ECS::GameUUID classGUID() { return recluseHash(#_class, sizeof(#_class)); } \
    static const char* className() { return #_class; } \
    virtual Recluse::ECS::GameUUID getClassGUID() const override { return classGUID(); } \
    virtual const char* getClassName() const override { return className(); }


//! AbstractSystem is the high level provision that oversees all
//! game components to their respect.
//! Systems are what hold the game logic in the world scene.
class R_PUBLIC_API AbstractSystem : public Serializable
{
public:

    virtual ~AbstractSystem() { }

    void setPriority(U32 priority) { m_priority = priority; }
    U32 getPriority() const { return m_priority; }

    // This system is required to update all components when necessary.
    void    updateComponents(const RealtimeTick& tick) { onUpdateComponents(tick); }

    // Clear all components in the game object.
    void   clearAll()                      { onClearAll(); }

    // Obtains the total number of allocated components in the 
    // system.
    virtual U32     getTotalComponents() const      = 0;

    ErrType         initialize()
    {
        return onInitialize();
    }

    virtual ErrType serialize(Archive* archive) override { return R_RESULT_NO_IMPL; }
    virtual ErrType deserialize(Archive* archive) override { return R_RESULT_NO_IMPL; }

private:
    virtual ErrType onInitialize()                  { return R_RESULT_NO_IMPL; }
    virtual ErrType onCleanUp()                     { return R_RESULT_NO_IMPL; }
    virtual void onClearAll()                       { }
    virtual void onUpdateComponents(const RealtimeTick& tick)  { }

    U32 m_priority;
};

// Required declare for the game system to be used. 
// Use the constructor you feel is important.
// A Default destructor is required in order to do final cleanups at the end of an application's life.
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
    ErrType allocateComponent(Comp** pOut, GameEntity* pOwner)  
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

    // Available functions to query from the given system.
    virtual U32     getTotalComponents() const override { return m_numberOfComponentsAllocated; }

    // Get all components handled by the system. This is optional, so be sure to check if this is 
    // actually implemented.
    virtual Comp**  getAllComponents(U64& pOut) { return nullptr; }

    // Get a component from the system. Return nullptr, if the component doesn't match the given 
    // game entity key.
    virtual Comp*   getComponent(const RGUID& entityKey) { return nullptr; }

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

// Takes the global system for a given component, and casts it to System<>*
// Returns null if the system fo the given components are not available.
template<typename Comp>
static System<Comp>* castToSystem()
{
    return dynamic_cast<System<Comp>*>(Comp::getSystem());
}
} // ECS
} // Recluse