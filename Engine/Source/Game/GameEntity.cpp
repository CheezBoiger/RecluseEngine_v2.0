//
#include "Recluse/Game/GameEntity.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include <unordered_map>

namespace Recluse {
namespace ECS {


struct EntityAllocation
{
    GameEntity*             entity;
    GameEntityAllocation    memory;
};

std::unordered_map<RGUID, EntityAllocation, RGUID::Hash, RGUID::Equal> kEntityMap;

static GameEntity* defaultAlloc(U64 szBytes, GameEntityMemoryAllocationType type, const Recluse::RGUID& rguid)
{
    GameEntityAllocation allocation = { };

    R_ASSERT(szBytes == sizeof(GameEntity));

    if (type == GameEntityMemoryAllocationType_Dynamic)
    {
        allocation.offsetAddress    = (SizeT)malloc(szBytes);
        allocation.szBytes          = szBytes;
        allocation.allocType        = type;

        // Generate a rguid for this entity.
        
        RGUID guid      = rguid; 
        
        R_ASSERT_FORMAT(kEntityMap.find(rguid) == kEntityMap.end(), "Newly allocated entity can not have a similar RGUID identifer as another entity! Setting to default...");
        
        if (!rguid.isValid() || (kEntityMap.find(rguid) != kEntityMap.end())) 
        {
            guid = generateRGUID();
        }

        void* ptr       = reinterpret_cast<void*>(allocation.offsetAddress);
        
        GameEntity* entity = new (ptr) GameEntity(guid);
    
        kEntityMap.insert(std::make_pair(guid, EntityAllocation{ entity, allocation }));
        return entity;
    }
    else
    {
        R_ERROR("GameEntity", "Only Dynamic allocation is supported for default allocations!");
    }

    // Return null if we can't allocate the specified allocation type.
    return nullptr;
}

static void defaultFree(GameEntity* pEntity)
{
    R_ASSERT(pEntity != NULL);
    auto it = kEntityMap.find(pEntity->getGUID());
    if (it != kEntityMap.end())
    {
        GameEntityAllocation& allocation = it->second.memory;
        free((void*)allocation.offsetAddress);
        kEntityMap.erase(it);
    }
}


static GameEntity* defaultGetEntity(const RGUID& rguid)
{
    // Invalid guid, will return nullptr.
    if (rguid == RGUID::kInvalidValue)
        return nullptr;

    // Get the rguid hash.
    auto it = kEntityMap.find(rguid);
    if (it != kEntityMap.end())
    {
        return it->second.entity;
    }
    return nullptr;
}


R_INTERNAL void defaultCleanUpEntities()
{
    if (!kEntityMap.empty())
    {
        for (auto& it : kEntityMap)
        {
            EntityAllocation& entity = it.second;
            R_ASSERT(entity.entity);
            GameEntityAllocation allocation = entity.memory;
            free((void*)allocation.offsetAddress);
        }

        kEntityMap.clear();
    }
}


GameEntityAllocationCall gameEntityAllocator { defaultCleanUpEntities, nullptr, defaultAlloc, defaultFree, defaultGetEntity };


GameEntity* GameEntity::instantiate(U64 szBytes, GameEntityMemoryAllocationType allocType, const RGUID& rguid)
{
    return gameEntityAllocator.onAllocationFn(szBytes, allocType, rguid);
}


void GameEntity::free(GameEntity* entity)
{
    gameEntityAllocator.onFreeFn(entity);
}


void GameEntity::setOnAllocation(GameEntityAllocationCall callback)
{
    R_ASSERT(callback.onAllocationFn != nullptr || callback.onFreeFn != nullptr);
    gameEntityAllocator = callback;
}


GameEntity* GameEntity::findEntity(const RGUID& guid)
{
    return gameEntityAllocator.onFindEntityByRguidFn(guid);
}


void GameEntity::freeAll()
{
    gameEntityAllocator.onCleanUpFn();
}


ResultCode GameEntity::serialize(Archive* pArchive) const
{
    // TODO: Need to figure out how to obtain a proper rguid.
    //       Parent needs to also be included as well!
    RGUID guid = getGUID();
    pArchive->write((void*)&guid, sizeof(RGUID));

    const std::string& s    = getName();
    U64 szBytes             = s.size();
    const char* str         = s.c_str();

    pArchive->write((void*)str, szBytes);

    const std::string tag   = getTag();
    szBytes                 = tag.size();
    str                     = tag.c_str();

    pArchive->write((void*)str, szBytes);

    return RecluseResult_Ok;
}

ResultCode GameEntity::deserialize(Archive* pArchive)
{
    return RecluseResult_NoImpl;
}


Bool EntityHierarchy::exists(const RGUID& node)
{
    auto it = m_hierarchy.find(node);
    return (it != m_hierarchy.end());
}


RGUID EntityHierarchy::getParent(const RGUID& node)
{
    RGUID parent = RGUID();
    if (exists(node))
    {
        parent = m_hierarchy[node].parent;
    }
    return parent;
}


Bool EntityHierarchy::isChildOf(const RGUID& node, const RGUID& parent)
{
    Bool isChild = false;
    // Both need to exist in order for this to work.
    if (exists(node) && exists(parent))
    {
        // Check if the parent of node record is the same parent.
        isChild = !!(m_hierarchy[node].parent == parent);
    }
    return isChild;
}


Bool EntityHierarchy::isParentOf(const RGUID& node, const RGUID& child)
{
    Bool isParent = false;
    if (exists(node) && exists(child))
    {
        // Check if the child node records has this node as its parent.
        isParent = !!(m_hierarchy[child].parent == node);
    }
    return isParent;
}


ResultCode EntityHierarchy::add(const RGUID& node, const RGUID& parent)
{
    // Don't re-add if it already exists in the hierarchy.
    if (!exists(node))
        m_hierarchy.insert(std::make_pair(node, Relation()));
    
    if (node == parent)
        return RecluseResult_Failed;

    // If this node was added already, needs to be removed from the other parent!
    {
        RGUID prevParent = getParent(node);
        if (prevParent.isValid() && prevParent != parent)
        {
            m_hierarchy[prevParent].children.erase(node);
        }
    }

    m_hierarchy[node].parent = parent;

    // Insert node to root, if there is no parent.
    if (!m_hierarchy[node].parent.isValid())
    {
        m_roots.insert(node);
    }
    else
    {
        // Be sure to add this node to the parent's children struct.
        auto parentIt = m_hierarchy.find(parent);
        if (parentIt != m_hierarchy.end())
        {
            ChildrenDataStructure& children = parentIt->second.children;
            R_ASSERT_FORMAT(children.find(node) == children.end(), "Child is already added to this parent!");
            children.insert(node);
        }

        // Finally, check if this node was previously in root, needs to be removed now.
        {
            auto rootIt = m_roots.find(node);
            m_roots.erase(node);
        }
    }
    return RecluseResult_Ok;
}


ResultCode EntityHierarchy::addAsChildrenForEntity(const RGUID& parent, const RGUID* children, U32 numChildren)
{
    if (!parent.isValid())
    {
        return RecluseResult_InvalidArgs;
    }

    auto it = m_hierarchy.find(parent);
    if (it != m_hierarchy.end())
    {
        if (parent == it->second.parent)
        {
            R_FATAL_ERROR("EntityHierarchy", "This parent's parent should not be added as a child!! Returning Failed.");
            return RecluseResult_InvalidArgs;
        }

        for (U32 i = 0; i < numChildren; ++i)
        {
            RGUID child = children[i];
            R_ASSERT_FORMAT(parent != child, "Parent should not be adding itself as a child to itself!! (parent inception...)");
            // Make sure the child does not already have a parent.
            if (!getParent(child).isValid())
            {
                // If a child relation doesn't already exist, make one and add as parent.
                add(child, parent);
            }
            else
            {
                R_ERROR("EntityHierarchy", "Child already has a parent! Remove it from that other parent, before adding to this parent!!");
            }
        }
    }
    else
    {
        // No parent found, likely was not added.
        return RecluseResult_NotFound;
    }

    return RecluseResult_Ok;
}


ResultCode EntityHierarchy::removeAsChildrenForEntity(const RGUID& parent, const RGUID* children, U32 numChildren)
{
    if (!parent.isValid())
    {
        return RecluseResult_InvalidArgs;
    }

    auto it = m_hierarchy.find(parent);
    if (it != m_hierarchy.end())
    {
        ChildrenDataStructure& entityChildren = it->second.children;
        for (U32 i = 0; i < numChildren; ++i)
        {
            RGUID child = children[i];
            entityChildren.erase(child);
            // Find any record of this child, and remove its parent association.
            auto childIt = m_hierarchy.find(child);
            if (childIt != m_hierarchy.end())
            {
                Relation& relation = childIt->second;
                relation.parent = RGUID();
            }
        }
    }
    return RecluseResult_Ok;
}


ResultCode EntityHierarchy::remove(const RGUID& node, RemovalOption removalOption)
{
    if (!node.isValid())
    {
        return RecluseResult_InvalidArgs;
    }

    ResultCode result = RecluseResult_Failed;

    if (exists(node))
    {
        // If we are intending to remove, we shouldn't have his option.
        if (removalOption != RemovalOption_JustCheck)
        {
            Relation& relation = m_hierarchy[node];
            RGUID parent = relation.parent;

            if (removalOption == RemovalOption_DestroyWithChildren)
            {
                ChildrenDataStructure& children = m_hierarchy[node].children;
                for (const auto& child : children)
                {
                    remove(child, removalOption);
                }
            }
            else if (removalOption == RemovalOption_RemoveAllSubtree)
            {
                // Add children to the root.
                ChildrenDataStructure& children = m_hierarchy[node].children;
                for (const auto& child : children)
                {
                    add(child, RGUID());
                }
            }
            else
            {
                // Add to the existing parent.
                if (parent.isValid())
                {
                    ChildrenDataStructure& children = m_hierarchy[node].children;
                    for (const auto& child : children)
                    {
                        add(child, parent);
                    }
                }
            }

            // Remove the node parent child association.
            if (parent.isValid())
            {
                Relation& parentRelation = m_hierarchy[parent];
                ChildrenDataStructure& parentChildren = parentRelation.children;
                auto childIt = parentChildren.find(node);
                parentChildren.erase(childIt);
            }

            // Finally, remove the entity from the hierarchy.
            {
                // Remove if node was a root node.
                auto rootIt = m_roots.find(node);
                if (rootIt != m_roots.end())
                {
                    m_roots.erase(rootIt);
                }
            }

            auto it = m_hierarchy.find(node);
            m_hierarchy.erase(it);
        }

        result = RecluseResult_Ok;
    }

    return result;
}


U32 EntityHierarchy::getNumberOfChildrenOfEntity(const RGUID& entity)
{
    if (!entity.isValid())
        return 0u;

    U32 numberChildren = 0u;
    auto entityIt = m_hierarchy.find(entity);
    if (entityIt != m_hierarchy.end())
    {
        // Get the number of children.
        numberChildren = entityIt->second.children.size();
    }
    return numberChildren;
}


ResultCode EntityHierarchy::getChildrenOfEntity(const RGUID& parent, RGUID* childrenOut)
{
    if (!parent.isValid())
        return RecluseResult_InvalidArgs;

    auto entityIt = m_hierarchy.find(parent);
    if (entityIt != m_hierarchy.end())
    {
        ChildrenDataStructure& children = entityIt->second.children;
        U32 i = 0;
        for (const auto& child : children)
        {
            childrenOut[i++] = child;
        }
    }
    return RecluseResult_Ok;
}


ResultCode EntityHierarchy::getRootNodes(RGUID* nodes, U32 numRootNodes)
{
    return RecluseResult_NoImpl;
}


ResultCode EntityHierarchy::serialize(Archive* archive) const
{
    return RecluseResult_NoImpl;
}


ResultCode EntityHierarchy::deserialize(Archive* archive)
{
    return RecluseResult_NoImpl;
}
} // ECS
} // Recluse 