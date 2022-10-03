//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include "Recluse/Game/GameSystem.hpp"

namespace Recluse {
namespace ECS {

typedef Hash64 ComponentUUID;

// Declaration semantics used for editor.
#define R_EDITOR_DECLARE(varType, varName, varValue)

// Call this macro when declareing a component. This will be used by the engine to determine 
// the proper calls to be made to the GameObject.
#define R_COMPONENT_DECLARE(_class) \
    private: \
    static Recluse::ECS::System<_class>* k ## _class ## System; \
    public: \
    static Recluse::ECS::ComponentUUID classGUID() { return recluseHash(#_class, sizeof(#_class)); } \
    static R_PUBLIC_API _class* instantiate(Recluse::ECS::GameEntity* pOwner); \
    static R_PUBLIC_API ErrType free(_class* ); \
    virtual Recluse::ECS::ComponentUUID getClassGUID() const override { return _class::classGUID(); } \
    static R_PUBLIC_API Recluse::ECS::AbstractSystem* getSystem(); \
    static R_PUBLIC_API void systemInit();

// Call this macro to implement the declarations above! This is required to determine the right systems
// that will be called to allocate/dealloce and update the given components related to this!
// _game_system is the connecting piece to the given component implementation, it is the 
// system that will manage all of the components for this.
#define R_COMPONENT_IMPLEMENT(_class, _game_system) \
    Recluse::ECS::System<_class>* _class :: k ## _class ## System = nullptr; \
    _class* _class::instantiate(Recluse::ECS::GameEntity* pOwner) { \
        _class* pAllocatedComp = nullptr; \
        ErrType err = k ## _class ## System->allocateComponent(&pAllocatedComp, pOwner); \
        if (err != R_RESULT_OK) return nullptr; \
        return pAllocatedComp; \
    } \
    ErrType _class::free(_class* pComponent) { \
        return k ## _class ## System->freeComponent(&pComponent); \
    } \
    void _class::systemInit() { if (!k ## _class ## System) k ## _class ## System = _game_system::create(); } \
    Recluse::ECS::AbstractSystem* _class::getSystem() { return k ## _class ## System; }

// Component abstraction class.
class R_PUBLIC_API Component : public Serializable
{
protected:
    static const U32 kUpdateFlagZero = 0;
public:
    static const U64 unknownComponent = ~0u;

    // Default component construction.
    Component(RGUID guid, GameEntity* pOwner = nullptr)
        : m_pGameObject(pOwner)
        , m_enable(false)
        , m_cuuid(guid) { }

    virtual ~Component() { }

    virtual ErrType serialize(Archive* pArchive) override { return R_RESULT_NO_IMPL; }

    virtual ErrType deserialize(Archive* pArchive) override { return R_RESULT_NO_IMPL; }

    // Get the component owner.
    GameEntity*     getOwner() const { return m_pGameObject; }

    // Get the component uuid.
    RGUID           getUUID() const { return m_cuuid; }

    // Check if the component is enabled.
    R_EDITOR_DECLARE("visible", "public", true)
    Bool            isEnabled() const { return m_enable; }

    // Enable the component.
    R_EDITOR_DECLARE("visible", "public", true)
    void            setEnable(Bool enable) { m_enable = enable; if (enable) onEnable(); }

    // Initialize the component.
    void            initialize(GameEntity* pGameObject) 
    {
        if (!pGameObject) 
        {
            return;
        } 

        m_pGameObject = pGameObject;
        m_enable = true;

        onInitialize();
    }

    // clean up the component. Does not release it!
    void            cleanUp() 
    {
        m_enable = false;
        onCleanUp();
    }

    // An actual release of the component. This will require that this object be destroyed completely,
    // and freed back to the system.
    void            release()
    {
        cleanUp();
        onRelease();
    }

    void            setUpdateFlags(U32 updateFlags) { m_updateFlags = updateFlags; }

    U32             getUpdateFlags() const { return m_updateFlags; }

    void            clearUpdateFlags() { m_updateFlags = kUpdateFlagZero; }

    virtual ECS::ComponentUUID getClassGUID() const { return 0; }

    void            setOwner(GameEntity* pOwner) { m_pGameObject = pOwner; }

private:

    virtual void    onInitialize() { }
    virtual void    onCleanUp() { }
    virtual void    onEnable() { }

    // MANDATORY! We need to be able to release the component back to system memory.
    // By default you can call Component::free()
    virtual void    onRelease() = 0;

    // Game object owner of this component.
    GameEntity*     m_pGameObject;

    // The unique id of the component.
    RGUID           m_cuuid;

    // Flag whether this component is enabled or not.
    Bool            m_enable;

    U32             m_updateFlags;
};
} // ECS
} // Recluse