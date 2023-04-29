//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Time.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include "Recluse/Game/GameSystem.hpp"
#include "Recluse/RGUID.hpp"

namespace Recluse {
namespace ECS {

typedef Hash64  ComponentUUID;
typedef U32     ComponentUpdateFlags;

enum ComponentUpdateFlag
{
    ComponentUpdateFlag_None = 0
};

// Declaration semantics used for editor.
#define R_EDITOR_DECLARE(varType, varName, varValue)

// Call this macro when declareing a component. This will be used by the engine to determine 
// the proper calls to be made to the GameObject.
#define R_COMPONENT_DECLARE(_class) \
    private: \
    static Recluse::ECS::System<_class>* k ## _class ## System; \
    public: \
    static Recluse::ECS::ComponentUUID classGUID() { return recluseHashFast(#_class, sizeof(#_class)); } \
    static R_PUBLIC_API _class* instantiate(const RGUID& owner); \
    static R_PUBLIC_API ResultCode free(_class* ); \
    virtual Recluse::ECS::ComponentUUID getClassGUID() const override { return _class::classGUID(); } \
    static R_PUBLIC_API Recluse::ECS::AbstractSystem* getSystem(); \
    static R_PUBLIC_API void systemInit();

// Call this macro to implement the declarations above! This is required to determine the right systems
// that will be called to allocate/dealloce and update the given components related to this!
// _game_system is the connecting piece to the given component implementation, it is the 
// system that will manage all of the components for this.
#define R_COMPONENT_IMPLEMENT(_class, _game_system) \
    Recluse::ECS::System<_class>* _class :: k ## _class ## System = nullptr; \
    _class* _class::instantiate(const Recluse::RGUID& owner) { \
        _class* pAllocatedComp = nullptr; \
        ResultCode err = k ## _class ## System->allocateComponent(&pAllocatedComp, owner); \
        if (err != RecluseResult_Ok) return nullptr; \
        return pAllocatedComp; \
    } \
    ResultCode _class::free(_class* pComponent) { \
        return k ## _class ## System->freeComponent(&pComponent); \
    } \
    void _class::systemInit() { if (!k ## _class ## System) k ## _class ## System = _game_system::create(); } \
    Recluse::ECS::AbstractSystem* _class::getSystem() { return k ## _class ## System; }

// Component abstraction class. This is mainly a container holding 
// data that is to be processed or read by GameSystems. 
class R_PUBLIC_API Component : public Serializable
{
public:
    static const U64 unknownComponent = ~0u;

    // Default component construction.
    Component(RGUID guid, const RGUID& ownerGuid = RGUID())
        : m_ownerGuid(ownerGuid)
        , m_enable(false)
        , m_cuuid(guid) { }

    virtual ~Component() { }

    virtual ResultCode serialize(Archive* pArchive) override { return RecluseResult_NoImpl; }

    virtual ResultCode deserialize(Archive* pArchive) override { return RecluseResult_NoImpl; }

    // Get the component owner.
    RGUID     getOwner() const { return m_ownerGuid; }

    // Get the component uuid.
    RGUID           getUUID() const { return m_cuuid; }

    // Check if the component is enabled.
    R_EDITOR_DECLARE("visible", "public", true)
    Bool            isEnabled() const { return m_enable; }

    // Enable the component.
    R_EDITOR_DECLARE("visible", "public", true)
    void            setEnable(Bool enable) { m_enable = enable; if (enable) onEnable(); }

    // Initialize the component.
    void            initialize() 
    {
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

    // Set update flags for the given component. The values are mainly specific to the system itself.
    // 0 is always a clear value.
    void                        setUpdateFlags(ComponentUpdateFlags updateFlags) { m_updateFlags = updateFlags; }

    // Get the given update flags.
    ComponentUpdateFlags        getUpdateFlags() const { return m_updateFlags; }

    // Clear all update flags from the component.
    void                        clearUpdateFlags() { m_updateFlags = ComponentUpdateFlag_None; }

    virtual ECS::ComponentUUID  getClassGUID() const { return 0; }

    void                        setOwner(RGUID owner) { m_ownerGuid = owner; }

    // Check if the component has an owner.
    Bool                        hasOwner() const { return m_ownerGuid.isValid(); }

private:

    virtual void    onInitialize() { }
    virtual void    onCleanUp() { }
    virtual void    onEnable() { }

    // MANDATORY! We need to be able to release the component back to system memory.
    // By default you can call Component::free()
    virtual void    onRelease() = 0;

    // Game object owner of this component.
    RGUID                   m_ownerGuid;

    // The unique id of the component.
    RGUID                   m_cuuid;

    // Flag whether this component is enabled or not.
    Bool                    m_enable;

    // Component update flags used by the system itself.
    ComponentUpdateFlags    m_updateFlags;
};
} // ECS
} // Recluse