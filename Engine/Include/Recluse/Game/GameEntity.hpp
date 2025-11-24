//
#pragma once

#include "Recluse/Time.hpp"
#include "Recluse/Types.hpp"

#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Game/Component.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Utility.hpp"

#include "Recluse/Game/GameSystem.hpp"

#include "RecluseEngine_exports.hpp"

#include <map>
#include <set>
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
// general purpose info that identifies it in the world. It does not hold any information
// about parent or children that it may be linked to. The EntityHierarchy should be holding this info.
class GameEntity : public Serializable 
{
public:
    virtual ~GameEntity() { }

    RecluseEngine_PUBLIC_API GameEntity
            (
                const GameEntityAllocation& allocation,
                const RGUID& uuid,
                const std::string& tag = std::string(), 
                const std::string& name = std::string()
            )
        : m_allocation(allocation)
        , m_guuid(uuid)
        , m_status(GameEntityStatus_Unused)
        , m_name(name)
        , m_tag(tag)
    {}

    // Instantiates a game object into the pool
    static RecluseEngine_PUBLIC_API GameEntity* instantiate(U64 szBytes, GameEntityMemoryAllocationType allocType = GameEntityMemoryAllocationType_Dynamic);
    
    // Frees a game object from the pool.
    static RecluseEngine_PUBLIC_API void        free(GameEntity* gameObject);

    // Optional call to override the game object allocation pool. If no custom allocations are desired,
    // will use default allocating instead (mainly malloc.)
    static RecluseEngine_PUBLIC_API void        setOnAllocation(GameEntityAllocationCall allocCallback);

    // Get an entity from the entity pool.
    static RecluseEngine_PUBLIC_API GameEntity* findEntity(const RGUID& guid);

    // Free all resources from the manager, this should be called at the end of all use for entities.
    static RecluseEngine_PUBLIC_API void        freeAll();

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
    RecluseEngine_PUBLIC_API void                   initialize() 
    {
        m_status = GameEntityStatus_Initialized;
    }
    
    // Wake the game object, this should be called when
    // game object is asleep, or first time initialized.
    RecluseEngine_PUBLIC_API void                   wake() 
    { 
        m_status = GameEntityStatus_Awake;
    }

    // Put game object to sleep.
    RecluseEngine_PUBLIC_API void                   sleep() 
    {
        m_status = GameEntityStatus_Asleep;
    }

    RecluseEngine_PUBLIC_API void                   destroy() 
    {
        m_status = GameEntityStatus_Destroyed;
    }

    virtual RecluseEngine_PUBLIC_API ResultCode     serialize(Archive* pArchive) const override;
    virtual RecluseEngine_PUBLIC_API ResultCode     deserialize(Archive* pArchive) override;

    // Get the game object name.
    const std::string&     getName() const { return m_name; }

    // Get the game object tag.
    RecluseEngine_PUBLIC_API const std::string&     getTag() const { return m_tag; }

    // Grab the game object status.
    RecluseEngine_PUBLIC_API GameEntityStatus       getStatus() const { return m_status; }

    // Check if the game entity is ready or active.
    RecluseEngine_PUBLIC_API Bool                   isReady() const { return (m_status == GameEntityStatus_Initialized) || (m_status == GameEntityStatus_Active); }

    RecluseEngine_PUBLIC_API RGUID                  getGUID() const { return m_guuid; }

    void setName(const std::string& newName) { m_name = newName; }
    void setTag(const std::string& newTag) { m_tag = newTag; }

    // Obtain the object allocation.
    RecluseEngine_PUBLIC_API GameEntityAllocation   getAllocation() const { return m_allocation; }

    // Get the component that is associated with this entity, from the given scene.
    // Many registries may hold components of the same entity, but may actually be from another registry.
    // Therefore, components of the same type will end up having different possible values from other registries.
    // To transition components to other registires, you will have to pull them from one scene and copy them to another.
    // Otherwise, to save memory, you can opt to call Scene::moveComponentToThis, which will detach the component from one registry, and 
    // Attach to another.
    template<typename Comp>
    Comp* getComponent(ECS::Registry* registry)
    {
        if (registry)
        {
            ECS::ComponentRegistry<Comp>* componentRegistry = registry->getComponentRegistry<Comp>();
            if (componentRegistry)
            {
                return componentRegistry->getComponent(getUUID());
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

    // Game Object uuid.
    RGUID                                                   m_guuid;

    // The actual game object allocation.
    GameEntityAllocation                                    m_allocation;
};


// EntityHierarchy is used to define the hierarchy of entities in a scene or system.
// Use this when handling and managing a system hierarchy. EntityHierarchy should not
// have any loops, as it is acyclic.
class RecluseEngine_PUBLIC_API EntityHierarchy : public Serializable
{
public:
    enum RemovalOption
    {
        // Removes only the node, maintains the subtree by adding children to the grandfather node.
        RemovalOption_RemoveOnlyNode         = (1 << 0),
        // Removes all the subtree, including children. Turns them parentless if they are not meant to be deleted.
        RemovalOption_RemoveAllSubtree      = (1 << 1),
        // Keeps the children, moves them over to childless root.
        RemovalOption_DeleteChildren          = (1 << 2),
    };
    typedef U32 RemovalOptionFlags;

    EntityHierarchy() { }
    ~EntityHierarchy() { }

    ResultCode          addAsChildrenForEntity(const RGUID& parent, const RGUID* children, U32 numChildren);
    ResultCode          removeAsChildrenForEntity(const RGUID& parent, const RGUID* children, U32 numChildren);

    ResultCode          addChild(const RGUID& parent, const RGUID& child);

    U32                 getNumberOfChildrenOfEntity(const RGUID& parent);
    ResultCode          getChildrenOfEntity(const RGUID& parent, RGUID* childrenOut);

    // Gets the parent of a node, if one exists. Otherwise, returns invalid value.
    RGUID               getParent(const RGUID& node);

    // Check if this node exists in this hierarchy.
    Bool                exists(const RGUID& node);

    // Check if this node is a possible child of parent.
    Bool                isChildOf(const RGUID& node, const RGUID& parent);

    // Check if this node is a possible parent of child.
    Bool                isParentOf(const RGUID& node, const RGUID& child);

    // Removes a node and any associated parent/children involved with it. Be sure to call this function if an entity
    // is going to be completely destroyed! Otherwise record will still be kept...
    ResultCode          remove(const RGUID& node, RemovalOptionFlags removalOption = RemovalOption_RemoveAllSubtree);

    // Adds a node to the hierarchy, this would check if the node is already a child, or parent of other children.
    // If not, will be a root node that is parentless.
    ResultCode          add(const RGUID& node, const RGUID& parent = RGUID());

    ResultCode          serialize(Archive* archive) const override;
    ResultCode          deserialize(Archive* archive) override;

    U32                 getNumberOfRootNodes() const { return m_roots.size(); }
    ResultCode          getRootNodes(RGUID* nodes, U32 numRootNodes);

    U32                 getFlatten() const;

private:
    // Children data structure.
    typedef std::set<RGUID, RGUID::Less> ChildrenDataStructure;


    struct Relation
    {
        ChildrenDataStructure   children;
        RGUID                   parent;  
    };

    std::map<RGUID, Relation, RGUID::Less>  m_hierarchy;

    // Nodes that are parentless. They are usually the top of the hierarchy, since they have no parent, likely root nodes.
    std::set<RGUID, RGUID::Less>            m_roots;

    // Cached hierarchy, no update needed.
    Bool m_cached;
};
} // ECS
} // Recluse