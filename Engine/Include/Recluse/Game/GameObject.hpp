//
#pragma once

#include "Recluse/RealtimeTick.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Game/Component.hpp"

#include <set>

namespace Recluse {
namespace Engine {

class Component;

typedef U64 GameUUID;

enum GameObjectStatus {
    GAME_OBJECT_STATUS_UNUSED,
    GAME_OBJECT_STATUS_INITIALIZING,
    GAME_OBJECT_STATUS_INITIALIZED,
    GAME_OBJECT_STATUS_AWAKE,
    GAME_OBJECT_STATUS_ACTIVE,
    GAME_OBJECT_STATUS_SLEEPING,
    GAME_OBJECT_STATUS_ASLEEP,
    GAME_OBJECT_STATUS_WAKING,
    GAME_OBJECT_STATUS_DESTROYED
};

// Game object, which holds all our object logic and script behavior.
// Kept as native C++, this class holds scripting behavior, which 
// will be used by objects in the game world.
class GameObject {
public:
    virtual ~GameObject() { }

    void initialize() {
        m_status = GAME_OBJECT_STATUS_INITIALIZING;
        onInitialize();
        m_status = GAME_OBJECT_STATUS_INITIALIZED;
    }
    
    void wake() { 
        m_status = GAME_OBJECT_STATUS_WAKING;
        onWake();
        m_status = GAME_OBJECT_STATUS_AWAKE;
    }

    void update(const RealtimeTick& tick) {
        m_status = GAME_OBJECT_STATUS_ACTIVE;
        onUpdate(tick);
    }

    void sleep() {
        m_status = GAME_OBJECT_STATUS_SLEEPING;
        onSleep();
        m_status = GAME_OBJECT_STATUS_ASLEEP;
    }

    void destroy() {
        onDestroy();
        m_status = GAME_OBJECT_STATUS_DESTROYED;
    }

    // Get the game object name.
    const std::string& getName() const { return m_name; }

    // Get the game object tag.
    const std::string& getTag() const { return m_tag; }

    // Grab the game object status.
    GameObjectStatus getStatus() const { return m_status; }

    // Get the game object parent.
    GameObject* getParent() const { return m_pParentNode; }

    GameUUID getUUID() const { return m_guuid; }

    // Add a node to this game object. This game object becomes the 
    // parent of pNode. Any game objects that are similar to this one,
    // will not be added as a child.
    void addChild(GameObject* pNode) { 

        // No need to do anything if this node already exists in this game object.
        if (pNode->getParent() == this) return;

        // Check if there is not already a parent node. If so, we will
        // remove it from it's original parent, and replace with this.
        if (pNode->m_pParentNode) {
            pNode->getParent()->removeChild(pNode);
        }
 
        pNode->m_pParentNode = this;
    
        m_childrenNodes.insert(pNode);
}

    // Removes a child from this game object.
    void removeChild(GameObject* pNode) {
        if (pNode->m_pParentNode != this) {
            // Can not remove this node if it doesn't belong to 
            // this game object.
            return;
        }

        auto iter = m_childrenNodes.find(pNode); 
        if (iter != m_childrenNodes.end()) {
            m_childrenNodes.erase(iter);
            pNode->m_pParentNode = nullptr;
        }
    }

protected:

    // Override on initialize for your game object script.
    virtual R_EXPORT void onInitialize() { }

    // Override on wake, to determine what to wake up in the game object.
    virtual R_EXPORT void onWake() { }

    // Override update for each update on the game object.
    virtual R_EXPORT void onUpdate(const RealtimeTick& tick) { }

    // Override on update fixed, for any fixed updates.
    virtual R_EXPORT void onUpdateFixed(const RealtimeTick& tick) { }

    // Override when game object sleeps.
    virtual R_EXPORT void onSleep() { }

    // Override when game object must clean up any manual objects.
    virtual R_EXPORT void onDestroy() { }

    // Game Object tag.
    std::string m_tag;

    // Game object name.
    std::string m_name;

private:
    GameObjectStatus m_status;

    // The Parent node that this game object may be associated to.
    GameObject* m_pParentNode;

    // All children game object nodes associated with this game object.
    // Keep in mind that any object children must also inherit the world transform from the 
    // game object parent.
    std::set<GameObject*> m_childrenNodes;

    // Game Object uuid.
    GameUUID m_guuid;
};
} // Engine
} // Recluse