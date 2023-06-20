//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Algorithms/Common.hpp"

#include <queue>

namespace Recluse {

namespace Internal {
enum RBDirection 
{ 
    RBDirection_Left, 
    RBDirection_Right,
    RBDirection_Count,
    RBDirection_Nil
};

enum RBColor 
{ 
    RBColor_Red, 
    RBColor_Black 
};
} // Internal

// Red-Black tree implementation, which mostly follows the rules set in wikipedia:
// https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
//
// Ideally Red-Black trees offer optimal, and performant, storage and retrieval of ordered information, along with a 
// guarantee that operations will complete within a known time (yes, that is verbatum to the wiki article... but it's true...)
// In order to achieve expectant performance, and to maintain the balancing nature of these data structures, Red-Black trees must 
// adhere to 5 requirements, such that these properties can benefit.
// 
// The rules are as such:
//  1. Every node is either Red or Black.
//  2. All NIL nodes are considered Black. (The root node is also black, but that can be omitted, root can by either red or black.)
//  3. A Red node does not have a Red child.
//  4. Every path from a given node to any of its descendent NIL nodes, must go through the same number of Black nodes.
//  5. If a node N has exactly ONE child, ite must be a Red child, because if it were Black, its NIL descendents would sit at a different 
//      Black depth than N's NIL child, violating requirement 4.
//
// Keep in mind that this Red-Black tree implementation relies on tail recursion to maintain in-place performance.
// 
template<typename Type, typename Comparer = CompareLess<Type>, typename _Allocator = MallocAllocator>
class RBTree
{
private:
    struct RBNode 
    {
        struct RBNode*      children[Internal::RBDirection_Count];
        struct RBNode*      parent;
        Internal::RBColor   color;
        Type                data;

        struct RBNode* left() { return children[Internal::RBDirection_Left]; }
        struct RBNode* right() { return children[Internal::RBDirection_Right]; }
    };

public:
    typedef Type*           TypePointer;
    typedef Type&           TypeReference;
    typedef const Type*     ConstantTypePointer;
    typedef const Type&     ConstantTypeReference;

    typedef RBNode*         RBNodePointer;
    typedef const RBNode*   ConstantRBNodePointer;
    typedef RBNode&         RBNodeReference;
    typedef const RBNode&   ConstantRBNodeReference;

    RBTree(const _Allocator& alloc = _Allocator()) 
        : m_root(nullptr)
        , m_numNodes(0)
        , m_compare(Comparer())
        , m_alloc(alloc) { } 

    // Insert data to the tree.
    void                    insert(ConstantTypeReference data);

    // Remove from the tree.
    void                    remove(ConstantTypeReference data);

    //TypeReference           root() { return m_root->data; }
    TypePointer             root() { return &m_root->data; }
    ConstantTypePointer     root() const { return &m_root->data; }
    //ConstantTypeReference   root() const { return m_root->data; }

    Bool                    find(ConstantTypeReference data);

    U32                     size() const { return m_numNodes; }

    std::string             stringify() const;
    
private:

    void                    rotate(RBNodePointer node, Internal::RBDirection direction);
    void                    rotateLeft(RBNodePointer node) { rotate(node, Internal::RBDirection_Left); }
    void                    rotateRight(RBNodePointer node) { rotate(node, Internal::RBDirection_Right); }

    void                    insertFixup(RBNodePointer n);
    void                    insertFixup0(RBNodePointer n);
    void                    insertFixup1(RBNodePointer n);
    void                    insertFixup2(RBNodePointer n);
    void                    insertFixup3(RBNodePointer n);

    void                    removeFixup1(RBNodePointer n);
    void                    removeFixup2(RBNodePointer n);
    void                    removeFixup3(RBNodePointer n);
    void                    removeFixup4(RBNodePointer n);
    void                    removeFixup5(RBNodePointer n);

    RBNodePointer           findInOrderSuccessor(RBNodePointer node);
    void                    replaceNode(RBNodePointer oldNode, RBNodePointer newNode);
    Internal::RBColor       nodeColor(RBNodePointer node) const { return (node ? node->color : Internal::RBColor_Black); }
    RBNodePointer           nodeLookUp(ConstantTypeReference data);
    RBNodePointer           findInsertionNode(RBNodePointer initNode, ConstantTypeReference data, Internal::RBDirection& childDirection);

    // Obtain the node uncle.
    RBNodePointer           uncle(RBNodePointer node);

    // Obtain the sibling of the current node.
    RBNodePointer           sibling(RBNodePointer node);

    // Obtain the grandparent node.
    RBNodePointer           grandparent(RBNodePointer node);
    Bool                    isFull(RBNodePointer node) const { return (node->children[Internal::RBDirection_Left] && node->children[Internal::RBDirection_Right]); };
    Bool                    isChildless(RBNodePointer node) const { return (!node->children[Internal::RBDirection_Left] && !node->children[Internal::RBDirection_Right]); }

    // find the child direction of parent node. This assumes that parent exists, and is not NULL!!!
    Internal::RBDirection   childDirection(RBNodePointer node) const;

    RBNodePointer           makeNode(ConstantTypeReference data);
    void                    freeNode(RBNodePointer remNode);

    _Allocator              m_alloc;
    MemoryArena             m_arena;
    
    Comparer                m_compare;
    RBNodePointer           m_root;
    U32                     m_numNodes;
};


template<typename Type, typename Comparer, typename _Allocator>
Internal::RBDirection RBTree<Type, Comparer, _Allocator>::childDirection(RBNodePointer node) const
{
    if (node->parent->children[Internal::RBDirection_Left] == node)
        return Internal::RBDirection_Left;
    else
        return Internal::RBDirection_Right;
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* RBTree<Type, Comparer, _Allocator>::makeNode(ConstantTypeReference data)
{
    RBNodePointer newNode = (RBNodePointer)m_alloc.allocate(sizeof(struct RBNode), pointerSizeBytes());
    newNode->color                                  = Internal::RBColor_Red;
    newNode->parent                                 = nullptr;
    newNode->children[Internal::RBDirection_Left]   = nullptr;
    newNode->children[Internal::RBDirection_Right]  = nullptr;
    newNode->data                                   = data;
    return newNode;
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::freeNode(RBNodePointer remNode)
{
    m_alloc.free((UPtr)remNode);
}

template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::insertFixup3(RBNodePointer node)
{
    RBNodePointer gp = grandparent(node);
    node->parent->color = Internal::RBColor_Black;  
    gp->color = Internal::RBColor_Red;
    if (node == node->parent->left())
        rotateRight(gp);
    else
        rotateLeft(gp);
}

template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::insertFixup2(RBNodePointer node)
{
    RBNodePointer gp = grandparent(node);
    if ((node == node->parent->right()) && 
        (node->parent == gp->left()))
    {
        rotateLeft(node->parent);
        node = node->left();
    }
    else if ((node == node->parent->left()) &&
             (node->parent == gp->right()))
    {
        rotateRight(node->parent);
        node = node->right();
    }
    insertFixup3(node);
}

template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::insertFixup1(RBNodePointer node)
{
    RBNodePointer u = uncle(node);
    RBNodePointer gp = nullptr;
    if (u && u->color == Internal::RBColor_Red)
    {
        node->parent->color = Internal::RBColor_Black;
        u->color = Internal::RBColor_Black;
        gp = grandparent(node);
        gp->color = Internal::RBColor_Red;
        insertFixup(gp);
    }
    else
        insertFixup2(node);
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::insertFixup0(RBNodePointer node)
{
    if (node->parent->color != Internal::RBColor_Black)
        insertFixup1(node);
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::insertFixup(RBNodePointer node)
{
    if (!node->parent)
        node->color = Internal::RBColor_Black;
    else
        insertFixup0(node);
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* 
RBTree<Type, Comparer, _Allocator>::findInsertionNode(RBNodePointer initNode, ConstantTypeReference data, Internal::RBDirection& childDirection)
{
    RBNodePointer prev = nullptr;
    while ( initNode )
    {
        prev = initNode;
        if (m_compare(data, initNode->data))
            childDirection = Internal::RBDirection_Left;
        else
            childDirection = Internal::RBDirection_Right;
        initNode = initNode->children[childDirection];
    }
    return prev;
}

template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::insert(ConstantTypeReference data)
{
    RBNodePointer newNode   = makeNode(data);

    if (!m_root)
    {
        m_root = newNode;
    }
    else
    {
        Internal::RBDirection direction = Internal::RBDirection_Nil;
        RBNodePointer parent = findInsertionNode(m_root, data, direction);
        parent->children[direction] = newNode;
        newNode->parent = parent;
    }

    m_numNodes += 1;
    insertFixup(newNode);
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::removeFixup5(RBNodePointer node)
{
    RBNodePointer sib = sibling(node);
    
    sib->color = node->parent->color;
    node->parent->color = Internal::RBColor_Black;
    
    if (node == node->parent->left())
    {
        if (nodeColor(sib->right()) == Internal::RBColor_Red)
        {
            sib->right()->color = Internal::RBColor_Black;
            rotateLeft(node->parent);
        }
    }
    else
    {
        if (nodeColor(sib->left()) == Internal::RBColor_Red)
        {
            sib->left()->color = Internal::RBColor_Black;
            rotateRight(node->parent);
        }
    }
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::removeFixup4(RBNodePointer node)
{
    RBNodePointer sib = sibling(node);
    if ((node == node->parent->left()) &&
        nodeColor(sib) == Internal::RBColor_Black &&
        nodeColor(sib->left()) == Internal::RBColor_Red &&
        nodeColor(sib->right()) == Internal::RBColor_Black)
    {
        sib->color = Internal::RBColor_Red;
        sib->left()->color = Internal::RBColor_Black;
        rotateRight(sib);
    }
    else if ((node == node->parent->right()) &&
             nodeColor(sib) == Internal::RBColor_Black &&
             nodeColor(sib->right()) == Internal::RBColor_Red &&
             nodeColor(sib->left()) == Internal::RBColor_Black)
    {
        sib->color = Internal::RBColor_Red;
        sib->right()->color = Internal::RBColor_Black;
        rotateLeft(sib);
    }
    removeFixup5(node);
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::removeFixup3(RBNodePointer node)
{
    RBNodePointer sib = sibling(node);
    if (nodeColor(node->parent) == Internal::RBColor_Red &&
        nodeColor(sib) == Internal::RBColor_Black &&
        nodeColor(sib->left()) == Internal::RBColor_Black &&
        nodeColor(sib->right()) == Internal::RBColor_Black)
    {
        sib->color = Internal::RBColor_Red;
        node->parent->color = Internal::RBColor_Black;
    }
    else
        removeFixup4(node);
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::removeFixup2(RBNodePointer node)
{
    RBNodePointer sib = sibling(node);
    if (nodeColor(node->parent) == Internal::RBColor_Black &&
        nodeColor(sib) == Internal::RBColor_Black &&
        nodeColor(sib->left()) == Internal::RBColor_Black &&
        nodeColor(sib->right()) == Internal::RBColor_Black)
    {
        sib->color = Internal::RBColor_Red;
        removeFixup1(node->parent);
    }
    else
        removeFixup3(node);
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::removeFixup1(RBNodePointer n)
{
    if (n->parent)
    {
        RBNodePointer sib = sibling(n);
        if (nodeColor(sib) == Internal::RBColor_Red)
        {
            n->parent->color = Internal::RBColor_Red;
            sib->color = Internal::RBColor_Black;
            if (n == n->parent->left())
                rotateLeft(n->parent);
            else
                rotateRight(n->parent);
        }
        removeFixup2(n);
    }
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::remove(ConstantTypeReference data)
{
    RBNodePointer temp = nodeLookUp(data);

    if (temp)
    {
        RBNodePointer remNode = temp;
        if (isFull(remNode))
        {
            RBNodePointer successor = findInOrderSuccessor(remNode);
            remNode->data = successor->data;
            remNode = successor;
        }

        if (!remNode->left() || !remNode->right())
        {
            RBNodePointer child = ((!remNode->right()) ? remNode->left() : remNode->right());
            if (remNode->color == Internal::RBColor_Black)
            {
                remNode->color = nodeColor(child);
                removeFixup1(remNode);
            }
            replaceNode(remNode, child);
            freeNode(remNode);
            m_numNodes -= 1;
        }
    }
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::replaceNode(RBNodePointer oldNode, RBNodePointer newNode)
{
    if (!oldNode->parent)
        m_root = newNode;
    else
        oldNode->parent->children[childDirection(oldNode)] = newNode; 

    if (newNode)
        newNode->parent = oldNode->parent;
}


template<typename Type, typename Comparer, typename _Allocator>
void RBTree<Type, Comparer, _Allocator>::rotate(RBNodePointer node, Internal::RBDirection direction)
{
    RBNodePointer gp    = node->parent;
    RBNodePointer s     = node->children[1 - direction];
    RBNodePointer c     = nullptr;

    c                               = s->children[direction];
    node->children[1 - direction]   = c;

    if (c)
        c->parent = node;

    s->children[direction]  = node;
    node->parent            = s;
    s->parent               = gp;

    if (gp)
        gp->children[node == gp->children[Internal::RBDirection_Right] ? Internal::RBDirection_Right : Internal::RBDirection_Left] = s;
    else
        m_root = s;
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* RBTree<Type, Comparer, _Allocator>::grandparent(RBNodePointer node)
{
    if (node && node->parent)
        return node->parent->parent;
    return nullptr;
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* RBTree<Type, Comparer, _Allocator>::uncle(RBNodePointer node)
{
    RBNodePointer gp = grandparent(node);
    if (!gp)
        return nullptr;
    return gp->children[(gp->children[Internal::RBDirection_Left] == node->parent) ? Internal::RBDirection_Right : Internal::RBDirection_Left];
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* RBTree<Type, Comparer, _Allocator>::sibling(RBNodePointer node)
{
    return node->parent->children[(node == node->parent->children[Internal::RBDirection_Left]) ? Internal::RBDirection_Right : Internal::RBDirection_Left];
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* RBTree<Type, Comparer, _Allocator>::findInOrderSuccessor(RBNodePointer node)
{
    RBNodePointer currNode = node->children[Internal::RBDirection_Right];
    while (currNode->children[Internal::RBDirection_Left])
        currNode = currNode->children[Internal::RBDirection_Left];
    return currNode;
}


template<typename Type, typename Comparer, typename _Allocator>
typename RBTree<Type, Comparer, _Allocator>::RBNode* RBTree<Type, Comparer, _Allocator>::nodeLookUp(ConstantTypeReference data)
{
    RBNodePointer currNode = m_root;
    while (currNode)
    {
        if (currNode->data == data)
            return currNode;
        
        if (m_compare(data, currNode->data))
            currNode = currNode->children[Internal::RBDirection_Left];
        else
            currNode = currNode->children[Internal::RBDirection_Right];
    }
    return currNode;
}


template<typename Type, typename Comparer, typename _Allocator>
std::string RBTree<Type, Comparer, _Allocator>::stringify() const
{
    std::string text;
    I32 items = 0;
    I32 levelLimit = 1;
    if (m_numNodes > 0)
    {
        std::queue<RBNode*> nodes;
        nodes.push(m_root);
        while (!nodes.empty())
        {
            RBNode* trav = nodes.front();
            nodes.pop();
            items++;
            if (items >= levelLimit)
            {
                text += "\n";
                levelLimit *= 2;
            }

            if (trav)
            {
                nodes.push(trav->children[Internal::RBDirection_Left]);
                nodes.push(trav->children[Internal::RBDirection_Right]);
                text += "[" + std::to_string(trav->data) + " " + (trav->color == Internal::RBColor_Black ? "BLACK" : "RED") + "]";
            }
            else
                text += "[NILL BLACK]";
        }
    }

    return text;
}
} // Recluse