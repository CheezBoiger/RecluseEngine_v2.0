//
#pragma once

#include "Recluse/Types.hpp"

#include <memory>
#include <unordered_map>

namespace Recluse {


// Lifetime cache manages the lifetime of objects based on a time tick. The tick is updated manually,
// but is used to determine the age of the objects that are contained. In the case of graphics, for each
// frame, we tick once and check the oldest resources, if it needs to be cleaned up. Any recently accessed
// resources are tagged to the recent tick, and pushed to the top of the list, which will then sort the oldest
// to the bottom. Any untagged resources, are left with the last tick they were accessed with, to which the last
// resource is cleaned up first.
template<typename IdentificationKey, typename Object>
class LifetimeCache
{
    // Lifetime node holds onto the data, as well as the key and age.
    // The age is tagged, every time it is accessed, to the current tick of this lifetime container.
    // Once tagged, it will be pushed to the top of the list as recently accessed.
    struct LifetimeNode
    {
        Object                  data;
        U32                     age;
        IdentificationKey       key;
        struct LifetimeNode*    next;
        struct LifetimeNode*    prev;

        LifetimeNode(IdentificationKey key, Object&& data = Object())
            : data(std::move(data))
            , age(0)
            , key(key)
            , next(nullptr)
            , prev(nullptr)
        { }
    };
public:

    LifetimeCache() 
        : m_root(nullptr)
        , m_tail(nullptr)
        , m_tick(0)
        , m_nodes(0)
    { }

    ~LifetimeCache()
    {
        clear();
    }

    // For each object in the cache. This will iterate through all objects.
    template<typename Func>
    void forEach(Func func)
    {
        LifetimeNode* current = m_root;
        while (current)
        {
            func(current->data);
            current = current->next;
        }
    }

    // Clear the resource cache. Mainly used if we need to clear all resources from this container.
    // Fully wipes out the structure.
    void clear()
    {
        LifetimeNode* current = m_root;
        while (current)
        {
            LifetimeNode* next = current->next;
            delete current;
            current = next;
        }
        m_cacheMap.clear();
        m_root = nullptr;
        m_tail = nullptr;
    }

    // Update the age tick for this container.
    void updateTick() { m_tick += 1; }

    // Checks the last resource, and destroys it if the age is too old.
    template<typename DeleteFunc>
    void check(U32 ageGap, DeleteFunc deleteFunc)
    {
        if (!empty())
        {
            LifetimeNode* tail = m_tail;
            const U32 tick = m_tick;
            const U32 ageRate = tick - tail->age;
            if (tail && (ageRate >= ageGap))
            {
                // Assign tail to its previous node.
                m_tail = tail->prev;
                // Cut off the tail of the linked list.
                if (tail != m_root)
                {
                    tail->prev->next = nullptr;
                }
                else
                {
                    // If it is the root, we must null both root and tail.
                    m_root = nullptr;
                    m_tail = nullptr;
                }
                // Delete the data, destroy the isolated node,
                // and decrement the number of nodes in the linked list.
                deleteFunc(tail->data);
                delete tail;
                m_nodes -= 1;
            }
        }
    }

    Bool inCache(IdentificationKey key)
    {
        return (m_cacheMap.find(key) != m_cacheMap.end());
    }

    // Refers to a resource in the cache.
    // Call this function first, before insert() to ensure we aren't creating 
    // duplicates.
    Object* refer(IdentificationKey key)
    {
        auto it = m_cacheMap.find(key);
        if (it != m_cacheMap.end())
        {
            pushFront(it->second);
            return &(it->second->data);
        }
        return nullptr;
    }

    // Inserts a new object into this container. Note that this will
    // blindly create a new resource in this container, so be sure to 
    // call refer() first, before inserting.
    Object* insert(IdentificationKey key, Object&& data)
    {
        // We need to create a new entry.
        LifetimeNode* node = new LifetimeNode(key);
        node->key = key;
        node->age = m_tick;
        node->data = std::move(data);
        if (!m_root)
        {
            // No root, means this is the first entry.
            m_root = node;
            m_tail = node;
        }
        else
        {
            // Push the node to the front of the list.
            pushFront(node);
        }
        m_cacheMap.insert(std::make_pair(key, node));
        m_nodes += 1;
        pushFront(node);
        return &(m_cacheMap[key]->data);
    }

    // Check if the cache is empty, no existing nodes in this container.
    Bool empty() const { return (m_nodes == 0); }

private:

    void pushFront(LifetimeNode* node)
    {
        if (!node) return;
        // Tag this node to the current tick.
        // If it is already root, ignore this call.
        // If not root, push this node to the front.
        node->age = m_tick;
        if (node != m_root)
        {
            // Assign the prev and next nodes of this node, to eachother.
            // This will isolate our current node.
            LifetimeNode* prev = node->prev;
            LifetimeNode* next = node->next;
            if (prev)
            {
                // Assign previous to the next node after this node,
                // Then check if this node is tail. If so, assign tail to previous.
                prev->next = next;
                if (node == m_tail)
                    m_tail = prev;
            }
            if (next)
                next->prev = prev;
            // Now push the node to the top.
            node->prev = nullptr;
            node->next = m_root;
            m_root->prev = node;
            m_root = node;
        }
    }
    // Map cache, used for O(1) access. Only holds onto weak references of the node.
    // Actual data is stored in linked list.
    std::unordered_map<IdentificationKey, LifetimeNode*> m_cacheMap;

    // The link list data structure. Houses the actual node and data associated.
    LifetimeNode*   m_root;
    LifetimeNode*   m_tail;
    U32             m_nodes;

    // Cache tick, used to determine the current state of the cache, and to check the age gap of 
    // any resources not accessed after a while.
    U32 m_tick;
};
} // Recluse