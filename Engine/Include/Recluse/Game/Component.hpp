//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Serializable.hpp"

namespace Recluse {

class GameObject;

typedef U64 ComponentUUID;

// Component abstraction class.
class Component : public Serializable {
public:
    // Default component construction.
    Component()
        : m_pGameObject(nullptr)
        , m_enable(false)
        , m_cuuid(0ull) { }

    virtual ~Component() { }

    ErrType serialize(Archive* pArchive) override { return REC_RESULT_NOT_IMPLEMENTED; }

    ErrType deserialize(Archive* pArchive) override { return REC_RESULT_NOT_IMPLEMENTED; }

    // Get the component owner.
    GameObject* getOwner() const { return m_pGameObject; }

    // Get the component uuid.
    ComponentUUID getUUID() const { return m_cuuid; }

    // Check if the component is enabled.
    B32 isEnabled() const { return m_enable; }

    // Enable the component.
    void setEnable(B32 enable) { m_enable = enable; if (enable) onEnable(); }

    // Initialize the component.
    void initialize(GameObject* pGameObject) {
        if (!pGameObject) {
            return;
        } 

        m_pGameObject = pGameObject;
        m_enable = true;

        onInitialize();
    }

    // clean up the component.
    void cleanUp() {
        m_enable = false;
        onCleanUp();
    }

    // update the component accordingly.
    virtual void update() { }

private:

    virtual void onInitialize() { }
    virtual void onCleanUp() { }
    virtual void onEnable() { }

    // Game object owner of this component.
    GameObject* m_pGameObject;

    // The unique id of the component.
    ComponentUUID m_cuuid;

    // Flag whether this component is enabled or not.
    B32 m_enable;
};
} // Recluse