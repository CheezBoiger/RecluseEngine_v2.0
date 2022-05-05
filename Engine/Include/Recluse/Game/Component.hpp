//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Serialization/Serializable.hpp"

#include "Recluse/Game/GameSystem.hpp"

namespace Recluse {
namespace ECS {

class GameObject;

typedef U64 ComponentUUID;

// Declaration semantics used for editor.
#define R_EDITOR_DECLARE(varType, varName, varValue)

// Component abstraction class.
class R_PUBLIC_API Component : public Serializable 
{
protected:
    static const U32 kUpdateFlagZero = 0;
public:
    static const U64 unknownComponent = ~0u;

    // Default component construction.
    Component()
        : m_pGameObject(nullptr)
        , m_enable(false)
        , m_cuuid(0ull) { }

    virtual ~Component() { }

    virtual ErrType serialize(Archive* pArchive) override { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType deserialize(Archive* pArchive) override { return REC_RESULT_NOT_IMPLEMENTED; }

    // Get the component owner.
    GameObject* getOwner() const { return m_pGameObject; }

    // Get the component uuid.
    ComponentUUID getUUID() const { return m_cuuid; }

    // Check if the component is enabled.
    R_EDITOR_DECLARE("visible", "public", true)
    Bool isEnabled() const { return m_enable; }

    // Enable the component.
    R_EDITOR_DECLARE("visible", "public", true)
    void setEnable(Bool enable) { m_enable = enable; if (enable) onEnable(); }

    // Initialize the component.
    void initialize(GameObject* pGameObject) 
    {
        if (!pGameObject) 
        {
            return;
        } 

        m_pGameObject = pGameObject;
        m_enable = true;

        onInitialize();
    }

    // clean up the component.
    void cleanUp() 
    {
        m_enable = false;
        onCleanUp();
    }

    // update the component accordingly. This must be defined for all components.
    void update(const RealtimeTick& tick)
    {   
        onUpdate(tick);
        clearUpdateFlags();
    }

    void setUpdateFlags(U32 updateFlags) { m_updateFlags = updateFlags; }

    U32 getUpdateFlags() const { return m_updateFlags; }

    void clearUpdateFlags() { m_updateFlags = kUpdateFlagZero; }

private:

    virtual void onUpdate(const RealtimeTick& tick) { }
    virtual void onInitialize() { }
    virtual void onCleanUp() { }
    virtual void onEnable() { }

    // Game object owner of this component.
    GameObject* m_pGameObject;

    // The unique id of the component.
    ComponentUUID m_cuuid;

    // Flag whether this component is enabled or not.
    Bool m_enable;

    U32 m_updateFlags;
};
} // ECS
} // Recluse