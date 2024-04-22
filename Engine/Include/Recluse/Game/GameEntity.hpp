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

class AbstractComponent;

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
typedef GameEntity* (*OnFindEntityByRguidCallback)   (const Recluse::RGUID&);
typedef void        (*OnCleanUpCallback)            ();
typedef void        (*OnInitializeCallback)         ();

struct GameEntityAllocationCall
{
    OnCleanUpCallback           onCleanUpFn;
    OnInitializeCallback        onInitializeFn;
    OnAllocationCallback        onAllocationFn;
    OnFreeCallback              onFreeFn;
    OnFindEntityByRguidCallback onFindEntityByRguidFn;
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
                RGUID parent = RGUID::kInvalidValue
            )
        : m_allocation(allocation)
        , m_guuid(uuid)
        , m_status(GameEntityStatus_Unused)
        , m_parent(parent)
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
    static R_PUBLIC_API GameEntity* findEntity(const RGUID& guid);

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
        m_status = GameEntityStatus_Destroyed;
    }

    virtual R_PUBLIC_API ResultCode     serialize(Archive* pArchive) const override;
    virtual R_PUBLIC_API ResultCode     deserialize(Archive* pArchive) override;

    // Get the game object name.
    R_PUBLIC_API const std::string&     getName() const { return m_name; }

    // Get the game object tag.
    R_PUBLIC_API const std::string&     getTag() const { return m_tag; }

    // Grab the game object status.
    R_PUBLIC_API GameEntityStatus       getStatus() const { return m_status; }

    // Check if the game entity is ready or active.
    R_PUBLIC_API Bool                   isReady() const { return (m_status == GameEntityStatus_Initialized) || (m_status == GameEntityStatus_Active); }

    // Get the game object parent.
    R_PUBLIC_API RGUID                  getParent() const { return m_parent; }

    R_PUBLIC_API RGUID                  getUUID() const { return m_guuid; }

    void setName(const std::string& newName) { m_name = newName; }
    void setTag(const std::string& newTag) { m_tag = newTag; }

    // Obtain the object allocation.
    R_PUBLIC_API GameEntityAllocation   getAllocation() const { return m_allocation; }

    // Add a node to this game object. This game object becomes the 
    // parent of pNode. Any game objects that are similar to this one,
    // will not be added as a child.
    R_PUBLIC_API void                   addChild(RGUID node) 
    { 
        // No need to do anything if this node already exists in this game object.
        GameEntity* entity = GameEntity::findEntity(node);
        if (!entity) return;
        if (entity->getParent() == m_guuid) return;

        // Check if there is not already a parent node. If so, we will
        // remove it from it's original parent, and replace with this.
        if (entity->getParent() != RGUID::kInvalidValue) 
        {
            GameEntity::findEntity(entity->getParent())->removeChild(node);
        }
 
        entity->m_parent = m_guuid;
    
        auto iter = std::find(m_children.begin(), m_children.end(), node);

        if (iter == m_children.end()) 
        {
            m_children.push_back(node);
        }
    }

    // Removes a child from this game object.
    R_PUBLIC_API void removeChild(RGUID node) 
    {
        GameEntity* entity = GameEntity::findEntity(node);
        if (!entity) return;

        if (entity->getParent() != m_guuid) 
        {
            // Can not remove this node if it doesn't belong to 
            // this game object.
            return;
        }

        auto iter = std::find(m_children.begin(), m_children.end(), node);
        if (iter != m_children.end()) 
        {
            m_children.erase(iter);
            entity->m_parent = RGUID::kInvalidValue;
        }
    }

    R_PUBLIC_API std::vector<RGUID>&  getChildren() { return m_children; }

    R_PUBLIC_API B32                        isParent(RGUID child) const 
    {
        auto iter = std::find(m_children.begin(), m_children.end(), child);
        return (iter != m_children.end());
    }

    // Get the component that is associated with this entity, from the given scene.
    // Many scenes may hold components of the same entity, but may actually be from another scene.
    // Therefore, components of the same type will end up having different possible values from other scenes.
    // To transition components to other scenes, you will have to pull them from one scene and copy them to another.
    // Otherwise, to save memory, you can opt to call Scene::moveComponentToThis, which will detach the component from one scene, and 
    // Attach to another.
    template<typename Comp>
    Comp* getComponent(Engine::Scene* pScene)
    {
        if (pScene)
        {
            ECS::Registry<Comp>* registry = pScene->getRegistry<Comp>();
            if (registry)
            {
                return registry->getComponent(getUUID());
            }
        }
        return nullptr;
    }

protected:

    // Game Object tag.
    std::string                                             m_tag;

    // Game object name.
   std::string                                              m_name;

private:

    struct GOCompCompare
    {
        bool operator ()(const ECS::AbstractComponent* lh, const ECS::AbstractComponent* rh) const
        {
            return lh->getClassGUID() < rh->getClassGUID();
        }
    };

    GameEntityStatus                                        m_status;

    // The Parent node that this game object may be associated to.
    RGUID                                                   m_parent;

    // All children game object nodes associated with this game object.
    // Keep in mind that any object children must also inherit the world transform from the 
    // game object parent.
    std::vector<RGUID>                                      m_children;

    // Game Object uuid.
    RGUID                                                   m_guuid;

    // The actual game object allocation.
    GameEntityAllocation                                    m_allocation;
};
} // ECS
} // Recluse