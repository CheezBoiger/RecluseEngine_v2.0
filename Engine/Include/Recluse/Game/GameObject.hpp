//
#pragma once

#include "Recluse/Time.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Game/Component.hpp"
#include "Recluse/Serialization/Hasher.hpp"

#include <set>
#include <vector>
#include <algorithm>

namespace Recluse {
namespace Engine {
class Scene;
} // Engine
} // Recluse

namespace Recluse {
namespace ECS {

class Component;

typedef U64 GameUUID;
typedef Hash64 RGUID;

enum GameObjectStatus 
{
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

#define R_PUBLIC_DECLARE_GAME_OBJECT(_class, guuid) \
    public: \
    static Recluse::ECS::RGUID classGUID() { return recluseHash(guuid, sizeof(guuid)); } \
    static const char* className() { return #_class; } \
    virtual Recluse::ECS::RGUID getClassGUID() const override { return classGUID(); } \
    virtual const char* getClassName() const override { return className(); }
    
    

// Game object, which holds all our object logic and script behavior.
// Kept as native C++, this class holds scripting behavior, which 
// will be used by objects in the game world.
// Essentially, this is our entity that contains the given 
// general purpose info that identifies it in the world.
class GameObject : public Serializable 
{
public:
    virtual ~GameObject() { }

    // Initialize the game object.
    R_PUBLIC_API void initialize() 
    {
        m_status = GAME_OBJECT_STATUS_INITIALIZING;
        onInitialize();
        m_status = GAME_OBJECT_STATUS_INITIALIZED;
    }
    
    // Wake the game object, this should be called when
    // game object is asleep, or first time initialized.
    R_PUBLIC_API void wake() 
    { 
        m_status = GAME_OBJECT_STATUS_WAKING;
        onWake();
        m_status = GAME_OBJECT_STATUS_AWAKE;
    }

    // Update the game object, per tick.
    R_PUBLIC_API void update(const RealtimeTick& tick) 
    {
        m_status = GAME_OBJECT_STATUS_ACTIVE;
        onUpdate(tick);
    }

    // Put game object to sleep.
    R_PUBLIC_API void sleep() 
    {
        m_status = GAME_OBJECT_STATUS_SLEEPING;
        onSleep();
        m_status = GAME_OBJECT_STATUS_ASLEEP;
    }

    R_PUBLIC_API void destroy() 
    {
        onDestroy();
        m_status = GAME_OBJECT_STATUS_DESTROYED;
    }

    virtual R_PUBLIC_API ErrType serialize(Archive* pArchive) override { return R_RESULT_NO_IMPL; }

    virtual R_PUBLIC_API ErrType deserialize(Archive* pArchive) override { return R_RESULT_NO_IMPL; }

    // Get the game object name.
    R_PUBLIC_API const std::string& getName() const { return m_name; }

    // Get the game object tag.
    R_PUBLIC_API const std::string& getTag() const { return m_tag; }

    // Grab the game object status.
    R_PUBLIC_API GameObjectStatus getStatus() const { return m_status; }

    // Get the game object parent.
    R_PUBLIC_API GameObject* getParent() const { return m_pParentNode; }

    R_PUBLIC_API GameUUID getUUID() const { return m_guuid; }

    // Add a node to this game object. This game object becomes the 
    // parent of pNode. Any game objects that are similar to this one,
    // will not be added as a child.
    R_PUBLIC_API void addChild(GameObject* pNode) 
    { 
        // No need to do anything if this node already exists in this game object.
        if (pNode->getParent() == this) return;

        // Check if there is not already a parent node. If so, we will
        // remove it from it's original parent, and replace with this.
        if (pNode->m_pParentNode) 
        {
            pNode->getParent()->removeChild(pNode);
        }
 
        pNode->m_pParentNode = this;
    
        auto iter = std::find(m_childrenNodes.begin(), m_childrenNodes.end(), pNode);

        if (iter == m_childrenNodes.end()) 
        {
            m_childrenNodes.push_back(pNode);
        }
    }

    // Removes a child from this game object.
    R_PUBLIC_API void removeChild(GameObject* pNode) 
    {
        if (pNode->m_pParentNode != this) 
        {
            // Can not remove this node if it doesn't belong to 
            // this game object.
            return;
        }

        auto iter = std::find(m_childrenNodes.begin(), m_childrenNodes.end(), pNode);
        if (iter != m_childrenNodes.end()) 
        {
            m_childrenNodes.erase(iter);
            pNode->m_pParentNode = nullptr;
        }
    }

    // Get the reference to the scene.
    //
    R_PUBLIC_API Engine::Scene* getScene() const { return m_pSceneRef; }

    R_PUBLIC_API std::vector<GameObject*>& getChildren() { return m_childrenNodes; }

    R_PUBLIC_API B32 isParent(GameObject* pChild) const 
    {
        auto iter = std::find(m_childrenNodes.begin(), m_childrenNodes.end(), pChild);
        return (iter != m_childrenNodes.end());
    }

    template<typename Comp>
    Comp* getComponent() { return nullptr; }

    template<typename Comp>
    void addComponent() { }

    virtual RGUID getClassGUID() const = 0;

    virtual const char* getClassName() const = 0;

protected:

    // Override on initialize for your game object script.
    virtual R_PUBLIC_API void onInitialize() { }

    // Override on wake, to determine what to wake up in the game object.
    virtual R_PUBLIC_API void onWake() { }

    // Override update for each update on the game object.
    virtual R_PUBLIC_API void onUpdate(const RealtimeTick& tick) { }

    // Override on update fixed, for any fixed updates.
    virtual R_PUBLIC_API void onUpdateFixed(const RealtimeTick& tick) { }

    // Override when game object sleeps.
    virtual R_PUBLIC_API void onSleep() { }

    // Override when game object must clean up any manual objects.
    virtual R_PUBLIC_API void onDestroy() { }

    // Game Object tag.
    std::string                 m_tag;

    // Game object name.
   std::string                  m_name;

private:

    GameObjectStatus            m_status;

    // The Parent node that this game object may be associated to.
    GameObject*                 m_pParentNode;

    // All children game object nodes associated with this game object.
    // Keep in mind that any object children must also inherit the world transform from the 
    // game object parent.
    std::vector<GameObject*>    m_childrenNodes;

    // All component handles associated with this entity.
    std::vector<ECS::Component*> m_components;

    // Game Object uuid.
    GameUUID                    m_guuid;

    // Reference to the scene.
    Engine::Scene*              m_pSceneRef;
};
} // ECS
} // Recluse