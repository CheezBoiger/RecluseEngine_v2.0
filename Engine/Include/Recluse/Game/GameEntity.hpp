//
#pragma once

#include "Recluse/Time.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Game/Component.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/RGUID.hpp"

#include "Recluse/Game/GameSystem.hpp"

#include <map>
#include <vector>
#include <algorithm>

#define R_PUBLIC_DECLARE_GAME_OBJECT(_class) R_PUBLIC_DECLARE_GAME_ECS(_class)

namespace Recluse {
namespace Engine {
class Scene;
} // Engine
} // Recluse

namespace Recluse {
namespace ECS {

class Component;

enum GameEntityStatus 
{
    GameEntityStatus_Unused,
    GameEntityStatus_Initializing,
    GameEntityStatus_Initialized,
    GameEntityStatus_Awake,
    GameEntityStatus_Active,
    GameEntityStatus_Sleeping,
    GameEntityStatus_Asleep,
    GameEntityStatus_Waking,
    GameEntityStatus_Destroyed
};


enum GameEntityMemoryAllocationType
{
    GameEntityMemoryAllocationType_Dynamic,                                             // 
    GameEntityMemoryAllocationType_Persistant,                                          // 
    GameEntityMemoryAllocationType_Static = GameEntityMemoryAllocationType_Persistant,  // 
    GameEntityMemoryAllocationType_Volatile                                             // Volatile/Transient memory type.
};

class GameEntity;


struct GameEntityAllocation
{
    U64                             offsetAddress;
    U64                             szBytes;
    GameEntityMemoryAllocationType  allocType;
};

// Game entity manager calls that need to be overridden if you plan to override the manager!
typedef GameEntity* (*OnAllocationCallback)         (U64, GameEntityMemoryAllocationType);
typedef void        (*OnFreeCallback)               (GameEntity*);
typedef GameEntity* (*OnGetEntityByRguidCallback)   (const Recluse::RGUID&);
typedef void        (*OnCleanUpCallback)            ();
typedef void        (*OnInitializeCallback)         ();

struct GameEntityAllocationCall
{
    OnCleanUpCallback           onCleanUpFn;
    OnInitializeCallback        onInitializeFn;
    OnAllocationCallback        onAllocationFn;
    OnFreeCallback              onFreeFn;
    OnGetEntityByRguidCallback  onGetEntityByRguidFn;
};

// Game entity, which holds all components associated with it.
// Kept as native C++.
// Essentially, this is our entity that contains the given 
// general purpose info that identifies it in the world.
class GameEntity : public Serializable 
{
public:
    virtual ~GameEntity() { }

    R_PUBLIC_API GameEntity
            (
                const GameEntityAllocation& allocation,
                const RGUID& uuid,
                const std::string& tag = std::string(), 
                const std::string& name = std::string(), 
                GameEntity* pParent = nullptr
            )
        : m_allocation(allocation)
        , m_guuid(uuid)
        , m_status(GameEntityStatus_Unused)
        , m_pParentNode(pParent)
        , m_pSceneRef(nullptr)
        , m_name(name)
        , m_tag(tag)
    {}

    // Instantiates a game object into the pool
    static R_PUBLIC_API GameEntity* instantiate(U64 szBytes, GameEntityMemoryAllocationType allocType = GameEntityMemoryAllocationType_Dynamic);
    
    // Frees a game object from the pool.
    static R_PUBLIC_API void        free(GameEntity* gameObject);

    // Optional call to override the game object allocation pool. If no custom allocations are desired,
    // will use default allocating instead (mainly malloc.)
    static R_PUBLIC_API void        setOnAllocation(GameEntityAllocationCall allocCallback);

    // Get an entity from the entity pool.
    static R_PUBLIC_API GameEntity* getEntity(const RGUID& guid);

    Bool isActive() const 
    {
        return (m_status == GameEntityStatus_Active);
    }

    // Activate the object.
    void activate()
    {
        initialize();
        wake();
        m_status = GameEntityStatus_Active;
    }

    // Initialize the game object.
    R_PUBLIC_API void                   initialize() 
    {
        m_status = GameEntityStatus_Initialized;
    }
    
    // Wake the game object, this should be called when
    // game object is asleep, or first time initialized.
    R_PUBLIC_API void                   wake() 
    { 
        m_status = GameEntityStatus_Awake;
    }

    // Put game object to sleep.
    R_PUBLIC_API void                   sleep() 
    {
        m_status = GameEntityStatus_Asleep;
    }

    R_PUBLIC_API void                   destroy() 
    {
        // Destroy our components involved here. This must be done LAST since these will end up invalid, and 
        // we won't be able to access them anymore here.
        for (auto& comp : m_components)
        {
            comp.second->release();
        }

        m_components.clear();

        m_status = GameEntityStatus_Destroyed;
    }

    virtual R_PUBLIC_API ErrType        serialize(Archive* pArchive) override;
    virtual R_PUBLIC_API ErrType        deserialize(Archive* pArchive) override;

    // Get the game object name.
    R_PUBLIC_API const std::string&     getName() const { return m_name; }

    // Get the game object tag.
    R_PUBLIC_API const std::string&     getTag() const { return m_tag; }

    // Grab the game object status.
    R_PUBLIC_API GameEntityStatus       getStatus() const { return m_status; }

    // Check if the game entity is ready or active.
    R_PUBLIC_API Bool                   isReady() const { return (m_status == GameEntityStatus_Initialized) || (m_status == GameEntityStatus_Active); }

    // Get the game object parent.
    R_PUBLIC_API GameEntity*            getParent() const { return m_pParentNode; }

    R_PUBLIC_API RGUID                  getUUID() const { return m_guuid; }

    void setName(const std::string& newName) { m_name = newName; }
    void setTag(const std::string& newTag) { m_tag = newTag; }

    // Obtain the object allocation.
    R_PUBLIC_API GameEntityAllocation   getAllocation() const { return m_allocation; }

    // Add a node to this game object. This game object becomes the 
    // parent of pNode. Any game objects that are similar to this one,
    // will not be added as a child.
    R_PUBLIC_API void                   addChild(GameEntity* pNode) 
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
    R_PUBLIC_API void removeChild(GameEntity* pNode) 
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
    R_PUBLIC_API Engine::Scene*             getScene() const { return m_pSceneRef; }

    R_PUBLIC_API std::vector<GameEntity*>&  getChildren() { return m_childrenNodes; }

    R_PUBLIC_API B32                        isParent(GameEntity* pChild) const 
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
            m_components.insert(std::pair<ECS::ComponentUUID, ECS::Component*>(Comp::classGUID(), Comp::instantiate(getUUID())));
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

protected:

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


    GameEntityStatus            m_status;

    // The Parent node that this game object may be associated to.
    GameEntity*                 m_pParentNode;

    // All children game object nodes associated with this game object.
    // Keep in mind that any object children must also inherit the world transform from the 
    // game object parent.
    std::vector<GameEntity*>    m_childrenNodes;

    // All component handles associated with this entity.
    std::map<Recluse::ECS::ComponentUUID, ECS::Component*> m_components;

    // Game Object uuid.
    RGUID                       m_guuid;

    // Reference to the scene.
    Engine::Scene*              m_pSceneRef;

    // The actual game object allocation.
    GameEntityAllocation        m_allocation;
};
} // ECS
} // Recluse