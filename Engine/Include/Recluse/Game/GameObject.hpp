//
#pragma once

#include "Recluse/Time.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Game/Component.hpp"
#include "Recluse/Serialization/Hasher.hpp"

#include "Recluse/Game/GameSystem.hpp"

#include <map>
#include <vector>
#include <algorithm>

#define R_PUBLIC_DECLARE_GAME_OBJECT(_class, guuid) R_PUBLIC_DECLARE_GAME_ECS(_class, guuid)

namespace Recluse {
namespace Engine {
class Scene;
} // Engine
} // Recluse

namespace Recluse {
namespace ECS {

class Component;

enum GameObjectStatus 
{
    GameObjectStatus_Unused,
    GameObjectStatus_Initializing,
    GameObjectStatus_Initialized,
    GameObjectStatus_Awake,
    GameObjectStatus_Active,
    GameObjectStatus_Sleeping,
    GameObjectStatus_Asleep,
    GAmeObjectStatus_Waking,
    GameObjectStatus_Destroyed
};

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
    R_PUBLIC_API void                   initialize() 
    {
        m_status = GameObjectStatus_Initializing;
        onInitialize();
        m_status = GameObjectStatus_Initialized;
    }
    
    // Wake the game object, this should be called when
    // game object is asleep, or first time initialized.
    R_PUBLIC_API void                   wake() 
    { 
        m_status = GAmeObjectStatus_Waking;
        onWake();
        m_status = GameObjectStatus_Awake;
    }

    // Update the game object, per tick.
    R_PUBLIC_API void                   update(const RealtimeTick& tick) 
    {
        m_status = GameObjectStatus_Active;
        onUpdate(tick);
    }

    // Put game object to sleep.
    R_PUBLIC_API void                   sleep() 
    {
        m_status = GameObjectStatus_Sleeping;
        onSleep();
        m_status = GameObjectStatus_Asleep;
    }

    R_PUBLIC_API void                   destroy() 
    {
        // We want to call this before, so that the user has a chance to collect any data from components.
        onDestroy();
        // Destroy our components involved here. This must be done LAST since these will end up invalid, and 
        // we won't be able to access them anymore here.
        for (auto& comp : m_components)
        {
            comp.second->release();
        }

        m_components.clear();

        m_status = GameObjectStatus_Destroyed;
    }

    virtual R_PUBLIC_API ErrType        serialize(Archive* pArchive) override { return R_RESULT_NO_IMPL; }
    virtual R_PUBLIC_API ErrType        deserialize(Archive* pArchive) override { return R_RESULT_NO_IMPL; }

    // Get the game object name.
    R_PUBLIC_API const std::string&     getName() const { return m_name; }

    // Get the game object tag.
    R_PUBLIC_API const std::string&     getTag() const { return m_tag; }

    // Grab the game object status.
    R_PUBLIC_API GameObjectStatus       getStatus() const { return m_status; }

    // Get the game object parent.
    R_PUBLIC_API GameObject*            getParent() const { return m_pParentNode; }

    R_PUBLIC_API GameUUID               getUUID() const { return m_guuid; }

    // Add a node to this game object. This game object becomes the 
    // parent of pNode. Any game objects that are similar to this one,
    // will not be added as a child.
    R_PUBLIC_API void                   addChild(GameObject* pNode) 
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
    Comp* getComponent() 
    { 
        static_assert(std::is_base_of<ECS::Component, Comp>(), "getComponent: Not base component!");
        auto& it = m_components.find(Comp::classGUID()); 
        if (it == m_components.end())
            return nullptr;
        return static_cast<Comp*>(it->second);
    }

    // Adds a component to the game object.
    template<typename Comp>
    Bool addComponent() 
    {
        static_assert(std::is_base_of<ECS::Component, Comp>(), "addComponent: Not base of component!");
        auto it = m_components.find(Comp::classGUID());
        if (it == m_components.end())
        {
            m_components.insert(std::pair<ECS::RGUID, ECS::Component*>(Comp::classGUID(), Comp::instantiate(this)));
            return true;
        }
        return false;
    }

    // Removes a component to the game object. Returns true if the component was successfully removed. False if the 
    // component does not exists in the game object.
    template<typename Comp>
    Bool removeComponent()
    {
        static_assert(std::is_base_of<ECS::Component, Comp>(), "removeComponent: Not a base of component!");
        auto it = m_components.find(Comp::classGUID());
        if (it == m_components.end())
            return false;
        it->second->release();
        m_components.erase(it);
        return true;
    }

    // get the class guid.
    virtual RGUID               getClassGUID() const = 0;

    // get the class name.
    virtual const char*         getClassName() const = 0;

protected:

    // Override on initialize for your game object script.
    virtual R_PUBLIC_API void   onInitialize() { }

    // Override on wake, to determine what to wake up in the game object.
    virtual R_PUBLIC_API void   onWake() { }

    // Override update for each update on the game object.
    virtual R_PUBLIC_API void   onUpdate(const RealtimeTick& tick) { }

    // Override on update fixed, for any fixed updates.
    virtual R_PUBLIC_API void   onUpdateFixed(const RealtimeTick& tick) { }

    // Override when game object sleeps.
    virtual R_PUBLIC_API void   onSleep() { }

    // Override when game object must clean up any manual objects.
    virtual R_PUBLIC_API void   onDestroy() { }

    // Game Object tag.
    std::string                 m_tag;

    // Game object name.
   std::string                  m_name;

private:

    struct GOCompCompare
    {
        bool operator ()(const ECS::Component* lh, const ECS::Component* rh) const
        {
            return lh->getClassGUID() < rh->getClassGUID();
        }
    };


    GameObjectStatus            m_status;

    // The Parent node that this game object may be associated to.
    GameObject*                 m_pParentNode;

    // All children game object nodes associated with this game object.
    // Keep in mind that any object children must also inherit the world transform from the 
    // game object parent.
    std::vector<GameObject*>    m_childrenNodes;

    // All component handles associated with this entity.
    std::map<Recluse::ECS::RGUID, ECS::Component*> m_components;

    // Game Object uuid.
    GameUUID                    m_guuid;

    // Reference to the scene.
    Engine::Scene*              m_pSceneRef;
};
} // ECS
} // Recluse